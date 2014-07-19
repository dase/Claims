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
	return true;
}

bool SortmergeJoin::next(BlockStreamBase *block){
	int total_length=state_._output_schema->getTupleMaxSize();
	void *joinedTuple=memalign(cacheline_size,state_._output_schema->getTupleMaxSize());
	int child_flag=0;
	void *tuple=0;
	remaining_context rc;
	if(atomicPopRemainingContext(rc)){
		while(1){
			if(child_flag==1){if(state_._child_left->next(rc.left_block_)==0){break;}}
			if(child_flag==2){if(state_._child_right->next(rc.right_block_)==0){break;}}
			for(left_tuple=0;(left_tuple=rc.left_iterator_->currentTuple())!=0;){
				for(right_tuple=0;(right_tuple=rc.right_iterator_->currentTuple())!=0;){
					/* 如果match了，输出到block中 */
					if(isMatch(left_tuple,right_tuple)){
						if((tuple=block->allocateTuple(total_length))>0){
						/* 复制 */
						}
					}
					else{
						/* 压入remaining_list */
						return true;
					}
					/* 如果不match，当left<right，外++ */

					/* 如果不match，当left>right，内++ */
				}
			}
		}
	}
	else{
		rc.left_block_=BlockStreamBase::createBlock(state_._input_schema_left);
		rc.right_block_=BlockStreamBase::createBlock(state_._input_schema_right);
		state_._child_left->next(rc.left_block_);
		state_._right_left->next(rc.right_block_);
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

bool SortmergeJoin::atomicPopRemainingContext(remaining_context &rc){
	lock_.acquire();
	if(remaining_context_list_.size()>0){
		rc=remaining_context_list_.front();
		remaining_context_list_.pop_front();
		lock_.release();
		return true;
	}
	else{
		lock_.release();
		return false;
	}
}

void SortmergeJoin::atomicPushRemainingContext(remaining_context rc){
	lock_.acquire();
	remaining_context_list_.push_back(rc);
	lock_.release();
}
