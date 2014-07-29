/*
 * SortedBlockStreamBuffer.cpp
 *
 *  Created on: 2014-7-28
 *      Author: casa
 */

#include "SortedBlockStreamBuffer.h"
#include "../../configure.h"

SortedBlockStreamBuffer::SortedBlockStreamBuffer(unsigned nPartitions, Schema *schema, unsigned blockSize, vector<unsigned> orderbyKey)
:nPartitions_(nPartitions),schema_(schema),blockSize_(blockSize),orderbyKey_(orderbyKey){

}

SortedBlockStreamBuffer::~SortedBlockStreamBuffer() {
	for(unsigned i=0;i<nPartitions_;i++){
		free(root_[i]);
	}
	TupleNode *p=sorted_root_;
	while(p!=0){
		free(p->tuple);
		free(p);
		p=p->next;
	}
}

void SortedBlockStreamBuffer::init(){
	for(unsigned i=0;i<nPartitions_;i++){
		TupleNode *root_index=(TupleNode *)malloc(sizeof(TupleNode));
		root_index->tuple=0;
		root_index->next=0;
		root_.push_back(root_index);
		cursor_.push_back(root_index);
	}
}

bool SortedBlockStreamBuffer::CompareTwoTuple(TupleNode *left, TupleNode* right) {
	int flag;
	const void *lt,*rt;
	for(unsigned i=0;i<orderbyKey_.size();i++){
		lt=schema_->getColumnAddess(orderbyKey_[i],left->tuple);
		rt=schema_->getColumnAddess(orderbyKey_[i],right->tuple);
		if(schema_->getcolumn(orderbyKey_[i]).operate->equal(lt,rt))
			flag=0;
		else if(schema_->getcolumn(orderbyKey_[i]).operate->less(lt,rt))
			flag=1;
		else
			flag=2;
		if(flag==0)
			continue;
		if(flag==1)
			return true;
		if(flag==2)
			return false;
	}
	return true;
}

TupleNode *SortedBlockStreamBuffer::mergeTwoList(TupleNode *left, TupleNode *right){
	TupleNode *ret=(TupleNode *)malloc(sizeof(TupleNode));
	ret->tuple=0;ret->next=0;
	TupleNode *r=ret;
	while(left!=0&&right!=0){
		/* compare the two tuple by using the schema. */
		if(CompareTwoTuple(left,right)){
			ret->next=left;
			ret=left;
			left=left->next;
		}
		else{
			ret->next=right;
			ret=right;
			right=right->next;
		}
	}
	while(left!=0){
		ret->next=left;
		ret=left;
		left=left->next;
	}
	while(right!=0){
		ret->next=right;
		ret=right;
		right=right->next;
	}
	return r->next;
}

void SortedBlockStreamBuffer::sortAllTuples() {
	/* merge all list, we write this like mergeTwoLists */
	TupleNode *p=root_[0]->next;
	for(unsigned i=1;i<root_.size();i++){
		p=mergeTwoList(p,root_[i]->next);
	}
	sorted_root_=p;
}

TupleNode* SortedBlockStreamBuffer::catAll(TupleNode *left, TupleNode *right) {
	TupleNode *ret=(TupleNode *)malloc(sizeof(TupleNode));
	ret->tuple=0;ret->next=0;
	TupleNode *r=ret;
	while(left!=0){
		ret->next=left;
		ret=left;
		left=left->next;
	}
	while(right!=0){
		ret->next=right;
		ret=right;
		right=right->next;
	}
	return r->next;
}

void SortedBlockStreamBuffer::insertIntoBuffer(BlockStreamBuffer *&buffer) {
	unsigned tupleSize=schema_->getTupleMaxSize();
	TupleNode *curse=sorted_root_;
	while(curse!=0){
		BlockStreamBase *block=BlockStreamBase::createBlock(schema_,blockSize_);
		void *desc=0;
		while((desc=block->allocateTuple(tupleSize))){
			memcpy(desc,curse->tuple,tupleSize);
			curse=curse->next;
			if(curse==0){
				buffer->insertBlock(block);
				return;
			}
		}
		buffer->insertBlock(block);
		free(block);
	}
}

void SortedBlockStreamBuffer::insertIntoSortedLists(BlockStreamBase *block, int index) {
	BlockStreamBase::BlockStreamTraverseIterator *itr=block->createIterator();
	void *tuple_in_block=0;
	TupleNode *p=cursor_[index];
	while((tuple_in_block=itr->nextTuple())!=0){
		TupleNode *node=(TupleNode *)malloc(sizeof(TupleNode));
		node->tuple=memalign(cacheline_size,schema_->getTupleMaxSize());
		memcpy(node->tuple,tuple_in_block,schema_->getTupleMaxSize());
		node->next=0;
		p->next=node;
		p=p->next;
	}
	cursor_[index]=p;
}
