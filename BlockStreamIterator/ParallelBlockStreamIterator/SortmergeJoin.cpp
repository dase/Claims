/*
 * SortmergeJoin.cpp
 *
 *  Created on: 2013-8-27
 *      Author: casa
 */

#include "SortmergeJoin.h"
SortmergeJoin::SortmergeJoin(State state)
:state_(state){

}

SortmergeJoin::SortmergeJoin(){

}

SortmergeJoin::~SortmergeJoin() {

}

bool SortmergeJoin::open(const PartitionOffset &partition_offset){
	state_._child_left->open(partition_offset);
	state_._child_right->open(partition_offset);
//	unsigned output=0;
//	for(unsigned i=0;i<state_._joinIndex_left.size();i++){
//		join_index_left_to_output_[i]=output;output++;}
//	for(unsigned i=0;i<state_._payload_left.size();i++){
//		join_payload_left_to_output_[i]=output;output++;}
//	for(unsigned i=0;i<state_._payload_right.size();i++){
//		join_payload_right_to_output_[i]=output;output++;}
	return true;
}

bool SortmergeJoin::next(BlockStreamBase *block){
	int total_length=state_._output_schema->getTupleMaxSize();
	void *joinedTuple=memalign(cacheline_size,state_._output_schema->getTupleMaxSize());
	int child_flag=0;
	void *tuple=0;void *left_tuple=0;void *right_tuple=0;
	remaining_context rc;
	if(atomicPopRemainingContext(rc)){
		while(1){
			if(child_flag==1){if(state_._child_left->next(rc.left_block_)==0){break;}}
			if(child_flag==2){if(state_._child_right->next(rc.right_block_)==0){break;}}
			for(;(left_tuple=rc.left_iterator_->currentTuple())!=0;){
				for(;(right_tuple=rc.right_iterator_->currentTuple())!=0;){
					/* 如果match了，输出到block中 */
					if(isMatch(left_tuple,right_tuple)){
						if((tuple=block->allocateTuple(total_length))>0){
							/* 组装 */
							assemble(left_tuple,right_tuple,joinedTuple);
							/* 复制 */
							state_._output_schema->copyTuple(joinedTuple,tuple);
						}
						else{
							/* 压入remaining_list */
							atomicPushRemainingContext(rc);
							return true;
						}
					}
					else{
						if(isleftLargerThanright(left_tuple,right_tuple)){
							/* 如果不match，当left>right，内++ */
							rc.right_iterator_->increase_cur_();
						}
						else{
							/* 如果不match，当left<right，外++ */
							rc.left_iterator_->increase_cur_();
						}
					}

				}
			}
		}
	}
	else{
		rc.left_block_=BlockStreamBase::createBlock(state_._input_schema_left,state_._block_size);
		rc.right_block_=BlockStreamBase::createBlock(state_._input_schema_right,state_._block_size);
		state_._child_left->next(rc.left_block_);
		state_._child_right->next(rc.right_block_);
		rc.left_iterator_=rc.left_block_->createIterator();
		rc.right_iterator_=rc.right_block_->createIterator();
		atomicPushRemainingContext(rc);
	}
}

bool SortmergeJoin::close(){

}

void SortmergeJoin::print(){

}

bool SortmergeJoin::isMatch(void *left, void *right){
	bool match=false;
	void *aleft;
	void *aright;
	for(unsigned i=0;i<state_._joinIndex_left.size();i++){
		aleft=state_._input_schema_left->getColumnAddess(state_._joinIndex_left[i],left);
		aright=state_._input_schema_right->getColumnAddess(state_._joinIndex_right[i],right);
		if(!state_._input_schema_left->getcolumn(state_._joinIndex_left[i]).operate->equal(aleft,aright)){
			match=false;break;}
		else{match=true;}}
	return match;
}

bool SortmergeJoin::isleftLargerThanright(void *left, void *right){
	bool llr=false;
	void *aleft;
	void *aright;
	for(unsigned i=0;i<state_._joinIndex_left.size();i++){
		aleft=state_._input_schema_left->getColumnAddess(state_._joinIndex_left[i],left);
		aright=state_._input_schema_right->getColumnAddess(state_._joinIndex_right[i],right);
		if(!state_._input_schema_left->getcolumn(state_._joinIndex_left[i]).operate->less(aleft,aright)){
			llr=false;break;}
		else{llr=true;}}
	return llr;
}

bool SortmergeJoin::atomicPopRemainingContext(SortmergeJoin::remaining_context &rc){
	lock_.acquire();
	if(remaining_context_list_.size()>0){
		rc=remaining_context_list_.front();
		remaining_context_list_.pop_front();
		lock_.release();
		return true;}
	else{
		lock_.release();return false;}
}

void SortmergeJoin::atomicPushRemainingContext(SortmergeJoin::remaining_context rc){
	lock_.acquire();
	remaining_context_list_.push_back(rc);
	lock_.release();
}

/*
 *   left schema: a1 a2 a3      a4 a5 a6
 *  right schema: b1 b2 b3      b4 b5 b6
 * output schema: a1 a2 a3      a4 a5 a6      b4 b5 b6
 * */
void SortmergeJoin::assemble(void *left, void *right, void *&assembledTuple){
	unsigned left_tuple_size=state_._input_schema_left->copyTuple(left,assembledTuple);
	state_._input_schema_left->copyTuple(right,assembledTuple+left_tuple_size);
}
