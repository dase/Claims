/*
 * BlockStreamSortIterator.cpp
 *
 *  Created on: 2014-1-18
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
:input_(0),orderbyKey_(0),child_(0),block_size_(0),partition_offset_(0){

}

BlockStreamSortIterator::State::State(Schema* input,unsigned orderbyKey,BlockStreamIteratorBase* child,const unsigned block_size,const PartitionOffset partitoin_offset)
:input_(input),orderbyKey_(orderbyKey),child_(child),block_size_(block_size),partition_offset_(partition_offset){

}

void BlockStreamSortIterator::order(DynamicBlockBuffer *dynamic_block_buffer,unsigned column){
	/* how to sort the data in the dynamic_block_buffer,
	 * it is not the simple case like sort the array of int
	 * */

}

bool BlockStreamSortIterator::open(const PartitionOffset& part_off){
	/* first we can store all the data which will be bufferred
	 * 1, buffer is the first phase. multi-threads will be applyed to the data
	 *    in the buffer.
	 * 2, sort the data in the buffer, we choose quicksort to sort the records
	 *    by specifying the column to be sorted
	 * 3, whether to register the buffer into the blockmanager.
	 * */
    BlockStreamBase* block_for_asking;

    state_.partition_offset_=part_off;

    state_.child_->open(Pthis->state_.partition_offset_);

    if(createBlockStream(block_for_asking)==false)

    /* phase 1: store the data in the buffer!
     *          by using multi-threads to speed up
     * */
    while(state_.child_->next(block_for_asking)){
		block_buffer_.atomicAppendNewBlock(block_for_asking);
		if(createBlockStream(block_for_asking)==false)
				return 0;
    }

    /* phase 2: sort the data in the buffer!
     *          by using multi-threads to speed up?
     * TODO: whether to store the sorted data into the blockmanager
     * */
    sort(block_buffer_,state_.orderbyKey_);

    return true;
}

bool BlockStreamSortIterator::next(BlockStreamBase* block){
	/* multi-threads to send the block out*/
    BlockStreamBase* buffered_block;
    while((buffered_block=(BlockStreamBase*)block_buffer_iterator_.atomicNextBlock())==0){
            if(ChildExhausted()){
				return false;
            }
            else{
				usleep(1);
            }
    }
    /* copy the buffered_block into block*/
    // block->deepCopy(buffered_block), this line must be exchanged!


    return true;
}

bool BlockStreamSortIterator::close(){
    DynamicBlockBuffer::Iterator it=block_buffer_.createIterator();
    BlockStreamBase* block_to_deallocate;
    while(block_to_deallocate=(BlockStreamBase*)it.nextBlock()){
		block_to_deallocate->~BlockStreamBase();
    }
    state_.child_->close();
    return true;
}


bool BlockStreamSortIterator::createBlockStream(BlockStreamBase*& target)const{
        //TODO: the block allocation should apply for the memory budget from the buffer manager first.
        target=BlockStreamBase::createBlock(state_.input_,state_.block_size_);
        return target!=0;
}

bool BlockStreamSortIterator::ChildExhausted(){
        return finished_thread_count_==registered_thread_count_;
}
