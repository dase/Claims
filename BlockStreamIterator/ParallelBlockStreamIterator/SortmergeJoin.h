/*
 * SortmergeJoin.h
 *
 *  Created on: 2014-7-17
 *      Author: casa
 */

#ifndef SORTMERGEJOIN_H_
#define SORTMERGEJOIN_H_

#include "../ExpandableBlockStreamIteratorBase.h"

class SortmergeJoin:public ExpandableBlockStreamIteratorBase{
public:
	class State{
		friend class BlockStreamJoinIterator;
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
			  );
		State(){};

		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & child_left & child_right & input_schema_left & input_schema_right & output_schema & joinIndex_left & joinIndex_right & payload_left & payload_right;
		}
	public:
		//input and output
		BlockStreamIteratorBase *child_left,*child_right;
		Schema *input_schema_left,*input_schema_right;
		Schema *output_schema;

		//how to join
		std::vector<unsigned> joinIndex_left;
		std::vector<unsigned> joinIndex_right;
		std::vector<unsigned> payload_left;
		std::vector<unsigned> payload_right;
	};
	SortmergeJoin(State state);
	SortmergeJoin();
	virtual ~SortmergeJoin();

	bool open(const PartitionOffset& partition_offset=0);
	bool next(BlockStreamBase *block);
	bool close();
	void print();

private:
	State state_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & boost::serialization::base_object<ExpandableBlockStreamIteratorBase>(*this) & state_;
    }
};

#endif /* SORTMERGEJOIN_H_ */
