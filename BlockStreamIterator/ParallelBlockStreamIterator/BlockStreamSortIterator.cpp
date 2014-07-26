/*
 * BlockStreamSortIterator.cpp
 *
 *  Created on: 2014-2-17
 *      Author: casa
 */

#include "BlockStreamSortIterator.h"

BlockStreamSortIterator::BlockStreamSortIterator(){
    sema_open_.set_value(1);
    sema_open_finished_.set_value(0);
}

BlockStreamSortIterator::BlockStreamSortIterator(State state)
:finished_thread_count_(0),registered_thread_count_(0),state_(state){
    sema_open_.set_value(1);
    sema_open_finished_.set_value(0);
}

BlockStreamSortIterator::~BlockStreamSortIterator(){

}

BlockStreamSortIterator::State::State()
:input_(0),orderbyKey_(0),child_(0),block_size_(0){

}

BlockStreamSortIterator::State::State(Schema* input,unsigned orderbyKey,BlockStreamIteratorBase* child,const unsigned block_size)
:input_(input),orderbyKey_(orderbyKey),child_(child),block_size_(block_size){

}

bool BlockStreamSortIterator::open(const PartitionOffset& part_off){
	//TODO: multi threads can be used to pipeline!!!
	swap_num=0;
	temp_cur=0;
	/* first we can store all the data which will be bufferred
	 * 1, buffer is the first phase. multi-threads will be applyed to the data
	 *    in the buffer.
	 * 2, sort the data in the buffer, we choose quicksort to sort the records
	 *    by specifying the column to be sorted
	 * 3, whether to register the buffer into the blockmanager.
	 * */
    BlockStreamBase* block_for_asking;

    state_.child_->open(part_off);

	if(sema_open_.try_wait()){
	block_buffer_iterator_=block_buffer_.createIterator();
		open_finished_ = true;
	}
	else{
		while(!open_finished_){
			usleep(1);
		}
	}

	root=(ListNode *)malloc(sizeof(ListNode));
	root->tuple=0;
	root->next=0;

	ListNode *p=root;

	if(createBlockStream(block_for_asking)==false)
    	;
    /* phase 1: store the data in the buffer!
     *          by using multi-threads to speed up
     * */
	void *tuple_ptr=0;
    BlockStreamBase::BlockStreamTraverseIterator *iterator_for_scan;
    while(state_.child_->next(block_for_asking)){
    	iterator_for_scan=block_for_asking->createIterator();
    	while((tuple_ptr=iterator_for_scan->nextTuple())!=0){
    		ListNode *node=(ListNode *)malloc(sizeof(ListNode));
    		node->tuple=tuple_ptr;
    		node->next=0;
    		p->next=node;
    		p=p->next;
    	}
		if(createBlockStream(block_for_asking)==false){
			cout<<"error in the create block stream!!!"<<endl;
			return 0;
		}
    }

    /* phase 2: sort the data in the buffer!
     *          by using multi-threads to speed up?
     * TODO: whether to store the sorted data into the blockmanager
     * */
    unsigned long long int time=curtick();
//    order();
	curse=cmsort(root->next);
    return true;
}

ListNode* BlockStreamSortIterator::findMiddle(ListNode *head){
	ListNode *fast=head;
	ListNode *slow=head;
	while(fast->next!=0&&fast->next->next!=0){
		fast=fast->next->next;
		slow=slow->next;
	}
	return slow;
}

ListNode *BlockStreamSortIterator::mergeTwoList(ListNode *left, ListNode *right){
	ListNode *ret=(ListNode *)malloc(sizeof(ListNode));
	ret->tuple=0;ret->next=0;
	ListNode *r=ret;
	const void *lt;
	const void *rt;
	while(left!=0&&right!=0){
		/* compare the two tuple by using the schema. */
		lt=state_.input_->getColumnAddess(state_.orderbyKey_[0],left->tuple);
		rt=state_.input_->getColumnAddess(state_.orderbyKey_[0],right->tuple);
//		cout<<"left and right: "<<*(int *)state_.input_->getColumnAddess(state_.orderbyKey_[0],l)<<" "<<*(int *)state_.input_->getColumnAddess(state_.orderbyKey_[0],r)<<endl;
		if(state_.input_->getcolumn(state_.orderbyKey_[0]).operate->less(lt,rt)){
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

/*
 * quick and slow pointer to find the middle node and merge sort.
 * */
ListNode *BlockStreamSortIterator::cmsort(ListNode *root){
	if(root==0||root->next==0)
		return root;
	ListNode *first=root;
	ListNode *middle=findMiddle(root);
	ListNode *second=middle->next;
	middle->next=0;
	ListNode *l1=cmsort(first);
	ListNode *l2=cmsort(second);
	return mergeTwoList(l1,l2);
}

bool BlockStreamSortIterator::next(BlockStreamBase* block){
	unsigned tuple_size=state_.input_->getTupleMaxSize();
	while(curse!=0){
		void *desc=0;
		while((desc=block->allocateTuple(tuple_size))){
			memcpy(desc,curse->tuple,tuple_size);
			curse=curse->next;
			if(curse!=0)
				continue;
			else
				break;
		}
		return true;
	}
	return false;

}

bool BlockStreamSortIterator::close(){
    state_.child_->close();
    return true;
}

bool BlockStreamSortIterator::createBlockStream(BlockStreamBase*& target)const{
        //TODO: the block allocation should apply for the memory budget from the buffer manager first.
        target=BlockStreamBase::createBlock(state_.input_,state_.block_size_);
        return target!=0;
}
