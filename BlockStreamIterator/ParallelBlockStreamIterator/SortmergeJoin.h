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
//		remaining_context(BlockStreamBase *left_block,BlockStreamBase *right_block,
//		BlockStreamBase::BlockStreamTraverseIterator *left_iterator,
//		BlockStreamBase::BlockStreamTraverseIterator *right_iterator)
//		:left_block_(left_block),
//		 right_block_(right_block),
//		 left_iterator_(left_iterator),
//		 right_iterator_(right_iterator){}
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
			  unsigned block_size
			  );
		State(){};
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & _child_left & _child_right & _input_schema_left & _input_schema_right & _output_schema & _joinIndex_left & _joinIndex_right;
		}
	public:
		//input and output
		BlockStreamIteratorBase *_child_left,*_child_right;
		Schema *_input_schema_left,*_input_schema_right;
		Schema *_output_schema;
		//how to join
		std::vector<unsigned> _joinIndex_left;
		std::vector<unsigned> _joinIndex_right;
		unsigned _block_size;
	};

	SortmergeJoin(State state);
	SortmergeJoin();
	virtual ~SortmergeJoin();

	bool open(const PartitionOffset& partition_offset=0);
	bool next(BlockStreamBase *block);
	bool close();
	void print();

	void assemble(void *, void *, void *&);

	bool isMatch(void*, void *);
	bool isleftLargerThanright(void *, void *);
	bool atomicPopRemainingContext(SortmergeJoin::remaining_context &rc);
	void atomicPushRemainingContext(SortmergeJoin::remaining_context rc);

private:
	State state_;
	Lock lock_;
	list<remaining_context> remaining_context_list_;

//	std::map<unsigned, unsigned> join_index_left_to_output_;
//	std::map<unsigned, unsigned> join_payload_left_to_output_;
//	std::map<unsigned, unsigned> join_payload_right_to_output_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & boost::serialization::base_object<ExpandableBlockStreamIteratorBase>(*this) & state_;
    }
};

#endif /* SORTMERGEJOIN_H_ */
