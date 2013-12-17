/*
 * ExpandableBlockStreamHistogramIterator.h
 *
 *  Created on: 2013-12-17
 *      Author: casa
 */

#ifndef EXPANDABLEBLOCKSTREAMHISTOGRAMITERATOR_H_
#define EXPANDABLEBLOCKSTREAMHISTOGRAMITERATOR_H_

#include "../../Debug.h"
#include "../../hashtable.h"
#include "../../Block/BlockStream.h"
#include "../../Catalog/Catalog.h"
#include "../../Catalog/Attribute.h"

#include <vector>
using namespace std;

class Statistics{
public:

private:
	int hash_histogram[HISTOGRAM_SIZE];
	int distinct_value_vol;
};

struct histogramPair{
	Attribute attribute;
	Statistics *statistics;
};

class ExpandableBlockStreamHistogramIterator {
public:
	class State{
		friend class ExpandableBlockStreamHistogramIterator;
	public:
		State(BlockStreamBase *child,
				Schema *schema,
				unsigned ht_nbuckets,
				unsigned block_size);
		State(){};
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & child_ & schema_ & ht_nbuckets_ & block_size_;
		}
	private:
		BlockStreamBase *child_;
		Schema *schema_;
		unsigned ht_nbuckets_;
		unsigned block_size_;
	};
	ExpandableBlockStreamHistogramIterator(State state){};
	ExpandableBlockStreamHistogramIterator();
	virtual ~ExpandableBlockStreamHistogramIterator();

	bool open(const PartitionOffset& partition_offset=0);
	bool next(BlockStreamBase *block);
	bool close();

private:
	State state_;
	BasicHashTable *hashtable_;
	vector<histogramPair> histogramPair_vector_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_;
    }
};

#endif /* EXPANDABLEBLOCKSTREAMHISTOGRAMITERATOR_H_ */
