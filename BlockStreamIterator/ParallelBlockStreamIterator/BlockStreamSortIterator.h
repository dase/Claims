/*
 * BlockStreamSortIterator.h
 *
 *  Created on: 2014-2-17
 *      Author: casa
 */

#ifndef BLOCKSTREAMSORTITERATOR_H_
#define BLOCKSTREAMSORTITERATOR_H_

#include <algorithm>
#include <iterator>
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include "../BlockStreamIteratorBase.h"
#include "../../common/Schema/Schema.h"
#include "../../configure.h"
#include "../../common/Block/BlockStream.h"
#include "../../common/Block/DynamicBlockBuffer.h"
#include "../../utility/rdtsc.h"

using namespace std;

struct ListNode{
	void *tuple;
	ListNode *next;
};

class BlockStreamSortIterator:public BlockStreamIteratorBase{
public:
	class State{
		friend class BlockStreamSortIterator;
	public:
		State();
		State(Schema* input,unsigned orderbyKey,BlockStreamIteratorBase* child,const unsigned block_size);
	public:
		//TODO: fixed and varible schema both needed!
		Schema* input_;
		//TODO:	orderby key can be any one of the column
		vector<unsigned> orderbyKey_;
		BlockStreamIteratorBase* child_;
		unsigned block_size_;
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & input_ & orderbyKey_ & child_ & block_size_;
		}
	};

	BlockStreamSortIterator();
	BlockStreamSortIterator(State state);
	virtual ~BlockStreamSortIterator();
	bool open(const PartitionOffset& part_off=0);
	bool next(BlockStreamBase* block);
	bool close();

private:
	ListNode *cmsort(ListNode *);
	ListNode *findMiddle(ListNode *);
	ListNode *mergeTwoList(ListNode *, ListNode *);
	bool createBlockStream(BlockStreamBase*&)const;

private:
	State state_;
	/* store the data in the buffer!*/
	DynamicBlockBuffer block_buffer_;
	/* create a iterator to traverse the buffer!*/
	DynamicBlockBuffer::Iterator block_buffer_iterator_;

	unsigned finished_thread_count_;
	unsigned registered_thread_count_;
	semaphore sema_open_;

	int swap_num;
	unsigned temp_cur;
	volatile bool open_finished_;
	semaphore sema_open_finished_;

	ListNode *root;
	ListNode *curse;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_ ;
	}
};

#endif /* BLOCKSTREAMSORTITERATOR_H_ */
