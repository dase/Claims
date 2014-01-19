/*
 * BlockStreamSortIterator.h
 *
 *  Created on: 2014-1-18
 *      Author: casa
 */

#ifndef BLOCKSTREAMSORTITERATOR_H_
#define BLOCKSTREAMSORTITERATOR_H_

#include "../BlockStreamIteratorBase.h"
#include "../../Schema/Schema.h"
#include "../../Block/DynamicBlockBuffer.h"

class BlockStreamSortIterator:public BlockStreamIteratorBase{

public:
	class State{
		friend class BlockStreamSortIterator;
	public:
		State(Schema* input,unsigned orderbyKey,BlockStreamIteratorBase* child,const unsigned block_size,const PartitionOffset partitoin_offset=0);
		State();
	private:
		/* now just the fixed type!*/
		Schema* input_;
		/* orderby key, this is the orderby key th*/
		unsigned orderbyKey_;
		BlockStreamIteratorBase* child_;
		unsigned block_size_;
		PartitionOffset partition_offset_;
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & input_ & orderbyKey_ & child_ & block_size_ & partition_offset_;
		}
	};

	BlockStreamSortIterator();
	BlockStreamSortIterator(State state);
	virtual ~BlockStreamSortIterator();
	bool open(const PartitionOffset& part_off=0);
	bool next(BlockStreamBase* block);
	bool close();

private:
	void order(DynamicBlockBuffer *dynamic_block_buffer,unsigned column);
	bool createBlockStream(BlockStreamBase*&)const;
	bool ChildExhausted();
private:
	State state_;
	/* store the data in the buffer!*/
	DynamicBlockBuffer block_buffer_;
	/* create a iterator to traverse the buffer!*/
	DynamicBlockBuffer::Iterator block_buffer_iterator_;

	unsigned finished_thread_count_;
	unsigned registered_thread_count_;
	semaphore sema_open_;

	volatile bool open_finished_;
	semaphore sema_open_finished_;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_ ;
	}
};

#endif /* BLOCKSTREAMSORTITERATOR_H_ */
