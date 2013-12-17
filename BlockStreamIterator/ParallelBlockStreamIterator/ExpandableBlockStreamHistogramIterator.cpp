/*
 * ExpandableBlockStreamHistogramIterator.cpp
 *
 *  Created on: 2013-12-17
 *      Author: casa
 */

#include "ExpandableBlockStreamHistogramIterator.h"

ExpandableBlockStreamHistogramIterator::ExpandableBlockStreamHistogramIterator() {
	// TODO 自动生成的构造函数存根

}

ExpandableBlockStreamHistogramIterator::~ExpandableBlockStreamHistogramIterator() {
	// TODO 自动生成的析构函数存根
}

ExpandableBlockStreamHistogramIterator::State::State(BlockStreamBase *child,
		Schema *schema,
		unsigned ht_nbuckets,
		unsigned block_size)
	:child_(child),
	 schema_(schema),
	 ht_nbuckets_(ht_nbuckets),
	 block_size_(block_size){

}

bool ExpandableBlockStreamHistogramIterator::open(const PartitionOffset& Partition_offset){

	return true;
}

bool ExpandableBlockStreamHistogramIterator::next(BlockStreamBase *block){

	return true;
}

bool ExpandableBlockStreamHistogramIterator::close(){

	return true;
}
