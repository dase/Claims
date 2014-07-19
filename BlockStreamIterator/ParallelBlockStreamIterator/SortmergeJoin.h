/*
 * SortmergeJoin.h
 *
 *  Created on: 2014-7-17
 *      Author: casa
 */

#ifndef SORTMERGEJOIN_H_
#define SORTMERGEJOIN_H_

#include "../ExpandableBlockStreamIteratorBase.h"
#include "../../configure.h"
#include "../../utility/lock.h"

#include <list>
using namespace std;

class SortmergeJoin:public ExpandableBlockStreamIteratorBase{
public:
	struct remaining_context{
		remaining_context()
		:left_block_(left_block),
		 right_block_(right_block),
		 left_iterator_(left_iterator),
		 right_iterator_(right_iterator){}
		BlockStreamBase *left_block_;BlockStreamBase *right_block_;
		BlockStreamBase::BlockStreamTraverseIterator *left_iterator_;
		BlockStreamBase::BlockStreamTraverseIterator *right_iterator_;
	};
	class State{
		friend class SortmergeJoin;
	public:
		State(BlockStreamIteratorBase *child_left,
			  BlockStreamIteratorBase *child_right,
			  Schema *input_schema_left,
			  Schema *input_schema_right,
			  Schema *output_schema,
			  std::vector<unsigned> joinIndex_left,
			  std::vector<unsigned> joinIndex_right,
			  std::vector<unsigned> payload_left,
			  std::vector<unsigned> payload_right,
			  unsigned block_size
			  ):_child_left(child_left),
				_child_right(child_right),
				_input_schema_left(input_schema_left),
				_input_schema_right(input_schema_right),
				_output_schema(output_schema),
				_joinIndex_left(joinIndex_left),
				_joinIndex_right(joinIndex_right),
				_payload_left(payload_left),
				_payload_right(payload_right),
				_block_size(block_size)
		{};
		State(){};

		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & child_left & child_right & input_schema_left & input_schema_right & output_schema & joinIndex_left & joinIndex_right & payload_left & payload_right;
		}
	public:
		//input and output
		BlockStreamIteratorBase *_child_left,*_child_right;
		Schema *_input_schema_left,*_input_schema_right;
		Schema *_output_schema;

		//how to join
		std::vector<unsigned> _joinIndex_left;
		std::vector<unsigned> _joinIndex_right;
		std::vector<unsigned> _payload_left;
		std::vector<unsigned> _payload_right;
		unsigned _block_size;
	};
	SortmergeJoin(State state);
	SortmergeJoin();
	virtual ~SortmergeJoin();

	bool open(const PartitionOffset& partition_offset=0);
	bool next(BlockStreamBase *block);
	bool close();
	void print();

	bool isMatch(void*, void *);
	bool atomicPopRemainingContext(remaining_context &rc);
	bool atomicPushRemainingContext(remaining_context rc);

private:
	State state_;
	Lock lock_;
	list<remaining_context> remaining_context_list_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & boost::serialization::base_object<ExpandableBlockStreamIteratorBase>(*this) & state_;
    }
};

#endif /* SORTMERGEJOIN_H_ */
