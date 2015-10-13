/*
 * ExpandableBlockStreamFilter.cpp
 *
 *  Created on: Aug 28, 2013
 *      Author: wangli
 */
#include "../physical_query_plan/ExpandableBlockStreamFilter.h"
#include <assert.h>
#include <limits>
#include "../../utility/warmup.h"
#include "../../utility/rdtsc.h"
#include "../../common/ExpressionCalculator.h"
#include "../../common/Expression/execfunc.h"
#include "../../common/Expression/qnode.h"
#include "../../common/Expression/initquery.h"
#include "../../common/Expression/queryfunc.h"
#include "../../common/data_type.h"
#include "../../Config.h"
#include "../../Parsetree/sql_node_struct.h"
#include "../../codegen/ExpressionGenerator.h"

#define NEWCONDITION

ExpandableBlockStreamFilter::ExpandableBlockStreamFilter(State state)
    : state_(state),
      generated_filter_function_(0),
      generated_filter_processing_fucntoin_(0) {
  InitExpandedStatus();
}

ExpandableBlockStreamFilter::ExpandableBlockStreamFilter()
    : generated_filter_function_(0), generated_filter_processing_fucntoin_(0) {
  InitExpandedStatus();
}

ExpandableBlockStreamFilter::~ExpandableBlockStreamFilter() {}
ExpandableBlockStreamFilter::State::State(Schema* schema,
                                          BlockStreamIteratorBase* child,
                                          vector<QNode*> qual,
                                          map<string, int> colindex,
                                          unsigned block_size)
    : schema_(schema),
      child_(child),
      qual_(qual),
      colindex_(colindex),
      block_size_(block_size) {}
ExpandableBlockStreamFilter::State::State(
    Schema* schema, BlockStreamIteratorBase* child,
    std::vector<AttributeComparator> comparator_list, unsigned block_size)
    : schema_(schema),
      child_(child),
      comparator_list_(comparator_list),
      block_size_(block_size) {}
bool ExpandableBlockStreamFilter::Open(const PartitionOffset& part_off) {
  RegisterExpandedThreadToAllBarriers();
  filter_thread_context* ftc =
      (filter_thread_context*)CreateOrReuseContext(crm_core_sensitive);

  if (TryEntryIntoSerializedSection()) {
    if (Config::enable_codegen) {
      ticks start = curtick();
      generated_filter_processing_fucntoin_ =
          getFilterProcessFunc(state_.qual_[0], state_.schema_);
      if (generated_filter_processing_fucntoin_) {
        printf("CodeGen (full feature) succeeds!(%f8.4ms)\n",
               getMilliSecond(start));
      } else {
        generated_filter_function_ =
            getExprFunc(state_.qual_[0], state_.schema_);
        if (generated_filter_function_) {
          ff_ = computeFilterwithGeneratedCode;
          printf("CodeGen (partial feature) succeeds!(%f8.4ms)\n",
                 getMilliSecond(start));
        } else {
          ff_ = computeFilter;
          printf("CodeGen fails!\n");
        }
      }
    } else {
      ff_ = computeFilter;
      printf("CodeGen closed!\n");
    }
  }
  bool ret = state_.child_->Open(part_off);
  SetReturnStatus(ret);
  BarrierArrive();
  return GetReturnStatus();
}

bool ExpandableBlockStreamFilter::Next(BlockStreamBase* block) {
  void* tuple_from_child;
  void* tuple_in_block;
  filter_thread_context* tc = (filter_thread_context*)GetContext();
  while (true) {
    if (tc->block_stream_iterator_->currentTuple() == 0) {
      /* mark the block as processed by setting it empty*/
      tc->block_for_asking_->setEmpty();
      if (state_.child_->Next(tc->block_for_asking_)) {
        delete tc->block_stream_iterator_;
        tc->block_stream_iterator_ = tc->block_for_asking_->createIterator();
      } else {
        if (!block->Empty())
          return true;
        else
          return false;
      }
    }
    process_logic(block, tc);
    /* there are totally two reasons for the end of the while loop.
     * (1) block is full of tuples satisfying filter (should return true to the
     * caller)
     * (2) block_for_asking_ is exhausted (should fetch a new block from child
     * and continue to process)
     */
    if (block->Full())
      // for case (1)
      return true;
  }
}
void ExpandableBlockStreamFilter::process_logic(BlockStreamBase* block,
                                                filter_thread_context* tc) {
  if (generated_filter_processing_fucntoin_) {
    int b_cur = block->getTuplesInBlock();
    int c_cur = tc->block_stream_iterator_->get_cur();
    const int b_tuple_count = block->getBlockCapacityInTuples();
    const int c_tuple_count = tc->block_for_asking_->getTuplesInBlock();

    generated_filter_processing_fucntoin_(
        block->getBlock(), &b_cur, b_tuple_count,
        tc->block_for_asking_->getBlock(), &c_cur, c_tuple_count);
    ((BlockStreamFix*)block)->setTuplesInBlock(b_cur);
    tc->block_stream_iterator_->set_cur(c_cur);
  } else {
    void* tuple_from_child;
    void* tuple_in_block;
    while ((tuple_from_child = tc->block_stream_iterator_->currentTuple()) >
           0) {
      bool pass_filter = true;
#ifdef NEWCONDITION
      ff_(pass_filter, tuple_from_child, generated_filter_function_,
          state_.schema_, tc->thread_qual_);
#else
      pass_filter = true;
      for (unsigned i = 0; i < state_.comparator_list_.size(); i++) {
        if (!state_.comparator_list_[i].filter(state_.schema_->getColumnAddess(
                state_.comparator_list_[i].get_index(), tuple_from_child))) {
          pass_filter = false;
          break;
        }
      }
#endif
      if (pass_filter) {
        const unsigned bytes =
            state_.schema_->getTupleActualSize(tuple_from_child);
        if ((tuple_in_block = block->allocateTuple(bytes)) > 0) {
          block->insert(tuple_in_block, tuple_from_child, bytes);
          tuple_after_filter_++;
        } else {
          /* we have got a block full of result tuples*/
          return;
        }
      }
      /* point the iterator to the next tuple */
      tc->block_stream_iterator_->increase_cur_();
    }
    /* mark the block as processed by setting it empty*/
    tc->block_for_asking_->setEmpty();
  }
}

bool ExpandableBlockStreamFilter::Close() {
  InitExpandedStatus();
  DestoryAllContext();
  state_.child_->Close();
  return true;
}

void ExpandableBlockStreamFilter::Print() {
  printf("filter: \n");
  for (int i = 0; i < state_.qual_.size(); i++) {
    printf("  %s\n", state_.qual_[i]->alias.c_str());
  }
  state_.child_->Print();
}

// void ExpandableBlockStreamFilter::destoryContext(thread_context& tc){
//	lock_.acquire();
//	assert(context_list_.find(pthread_self())!=context_list_.cend());
//	context_list_[pthread_self()].block_for_asking_->~BlockStreamBase();
//	context_list_[pthread_self()].iterator_->~BlockStreamTraverseIterator();
//	context_list_.erase(pthread_self());
//	lock_.release();
////	tc->block_for_asking_->~BlockStreamBase();
////	tc->iterator_->~BlockStreamTraverseIterator();
//}

void ExpandableBlockStreamFilter::computeFilter(bool& ret, void* tuple,
                                                expr_func func_gen,
                                                Schema* schema,
                                                vector<QNode*> thread_qual_) {
  ret = ExecEvalQual(thread_qual_, tuple, schema);
}

void ExpandableBlockStreamFilter::computeFilterwithGeneratedCode(
    bool& ret, void* tuple, expr_func func_gen, Schema* schema,
    vector<QNode*> allocator) {
  func_gen(tuple, &ret);
}

ExpandableBlockStreamFilter::filter_thread_context::~filter_thread_context() {
  delete block_for_asking_;
  delete temp_block_;
  delete block_stream_iterator_;
  for (int i = 0; i < thread_qual_.size(); i++) {
    delete thread_qual_[i];
  }
}

ThreadContext* ExpandableBlockStreamFilter::CreateContext() {
  filter_thread_context* ftc = new filter_thread_context();
  ftc->block_for_asking_ =
      BlockStreamBase::createBlock(state_.schema_, state_.block_size_);
  ftc->temp_block_ =
      BlockStreamBase::createBlock(state_.schema_, state_.block_size_);
  ftc->block_stream_iterator_ = ftc->block_for_asking_->createIterator();
  ftc->thread_qual_ = state_.qual_;
  for (int i = 0; i < state_.qual_.size(); i++) {
    Expr_copy(state_.qual_[i], ftc->thread_qual_[i]);
    InitExprAtPhysicalPlan(ftc->thread_qual_[i]);
  }
  return ftc;
}
