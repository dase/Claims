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

void BlockStreamSortIterator::swap(void *&desc,void *&src){
	swap_num++;
	void *temp=0;
	temp=desc;
	desc=src;
	src=temp;
}

void BlockStreamSortIterator::cssort(){
	stable_sort(secondaryArray_.begin(),secondaryArray_.end(),compare);
}

bool BlockStreamSortIterator::compare(const SNode *a,const SNode *b){
	const void *l=a->state_->input_->getColumnAddess(a->orderKey,a->tuple);
	const void *r=b->state_->input_->getColumnAddess(b->orderKey,b->tuple);
	return a->op->less(l,r);
}

void BlockStreamSortIterator::order(unsigned column,unsigned tuple_count){
	/* tranverse the buffer and apply the space to store the secondaryArray*/
}

void BlockStreamSortIterator::order(){
	for(unsigned i=0;i<state_.orderbyKey_.size();i++){
		Operate *op_=state_.input_->getcolumn(state_.orderbyKey_[i]).operate->duplicateOperator();
		for(unsigned j=0;j<secondaryArray_.size();j++){
			secondaryArray_[j]->orderKey=state_.orderbyKey_[i];
			secondaryArray_[j]->op=op_;
		}
		cssort();
	}
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
//    		cout<<"tuple: "<<*(int *)state_.input_->getColumnAddess(state_.orderbyKey_[0],p->tuple)<<endl;
//    		getchar();
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
    cmsort(root->next);
//	cout<<"the tuple_count is: "<<tuple_count_sum<<"Total time: "<<getSecond(time)<<" seconds, the swap num is: "<<swap_num<<endl;
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

//bool BlockStreamSortIterator::open(const PartitionOffset& part_off){
//	//TODO: multi threads can be used to pipeline!!!
//	swap_num=0;
//	temp_cur=0;
//	/* first we can store all the data which will be bufferred
//	 * 1, buffer is the first phase. multi-threads will be applyed to the data
//	 *    in the buffer.
//	 * 2, sort the data in the buffer, we choose quicksort to sort the records
//	 *    by specifying the column to be sorted
//	 * 3, whether to register the buffer into the blockmanager.
//	 * */
//    BlockStreamBase* block_for_asking;
//
//    state_.child_->open(part_off);
//
//	if(sema_open_.try_wait()){
//	block_buffer_iterator_=block_buffer_.createIterator();
//		open_finished_ = true;
//	}
//	else{
//		while(!open_finished_){
//			usleep(1);
//		}
//	}
//
//
//	if(createBlockStream(block_for_asking)==false)
//    	;
//    /* phase 1: store the data in the buffer!
//     *          by using multi-threads to speed up
//     * */
//    unsigned block_offset=0;
//    unsigned tuple_count_sum=0;
//	BlockStreamBase::BlockStreamTraverseIterator *iterator_for_scan;
//    while(state_.child_->next(block_for_asking)){
//    	tuple_count_sum+=block_for_asking->getTuplesInBlock();
//		block_buffer_.atomicAppendNewBlock(block_for_asking);
//		iterator_for_scan=block_buffer_.getBlock(block_offset)->createIterator();
//		void *tuple_ptr=0;
//		while((tuple_ptr=iterator_for_scan->nextTuple())!=0){
//			SNode *snode=(SNode*)malloc(sizeof(SNode));
//			snode->tuple=tuple_ptr;
//			snode->state_=&state_;
//			snode->op=0;
//			snode->orderKey=0;
//			secondaryArray_.push_back(snode);
//		}
//		block_offset++;
//		if(createBlockStream(block_for_asking)==false){
//			cout<<"error in the create block stream!!!"<<endl;
//			return 0;
//		}
//    }
//
//    /* phase 2: sort the data in the buffer!
//     *          by using multi-threads to speed up?
//     * TODO: whether to store the sorted data into the blockmanager
//     * */
//    unsigned long long int time=curtick();
//    order();
//	cout<<"the tuple_count is: "<<tuple_count_sum<<"Total time: "<<getSecond(time)<<" seconds, the swap num is: "<<swap_num<<endl;
//    return true;
//}

bool BlockStreamSortIterator::next(BlockStreamBase* block){
	unsigned tuple_size=state_.input_->getTupleMaxSize();
	ListNode *p=root->next;
	while(p!=0){
		void *desc=0;
		while((desc=block->allocateTuple(tuple_size))){
			memcpy(desc,p->tuple,tuple_size);
			p=p->next;
//    		cout<<"tuple: "<<*(int *)state_.input_->getColumnAddess(state_.orderbyKey_[0],p->tuple)<<endl;
//    		getchar();
			if(p!=0)
				continue;
			else
				break;
		}
		return true;
	}
	return false;

}

//bool BlockStreamSortIterator::next(BlockStreamBase* block){
//		/* multi-threads to send the block out*/
//	unsigned tuple_size=state_.input_->getTupleMaxSize();
//	while(temp_cur<secondaryArray_.size()){
//		void *desc=0;
//		while((desc=block->allocateTuple(tuple_size))){
//			memcpy(desc,secondaryArray_[temp_cur]->tuple,tuple_size);
//			temp_cur++;
//			if(temp_cur<secondaryArray_.size())
//				continue;
//			else
//				break;
//		}
//		return true;
//	}
//	return false;
//
//}

bool BlockStreamSortIterator::close(){
    state_.child_->close();
    return true;
}

bool BlockStreamSortIterator::createBlockStream(BlockStreamBase*& target)const{
        //TODO: the block allocation should apply for the memory budget from the buffer manager first.
        target=BlockStreamBase::createBlock(state_.input_,state_.block_size_);
        return target!=0;
}
