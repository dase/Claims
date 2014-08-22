/*
 * PartitionedShuffleBuffer.cpp
 *
 *  Created on: 2014-8-14
 *      Author: claims
 */

#include "PartitionedShuffleBuffer.h"

PartitionedShuffleBuffer::PartitionedShuffleBuffer(Schema *schema, unsigned blocksize, unsigned blocks, unsigned partitions)
:schema_(schema),block_size_(blocksize),n_blocks_(blocks),n_partitions_(partitions){
	for(unsigned i=0;i<n_blocks_;i++){
		BlockStreamBase *block=BlockStreamBase::createBlock(schema_,block_size_);
		empty_block_list_.push_back(block);
	}
	partitioned_block_list_=new list<BlockStreamBase *>[n_partitions_];
	count=0;
}

PartitionedShuffleBuffer::~PartitionedShuffleBuffer() {

}

bool PartitionedShuffleBuffer::insert(unsigned index, BlockStreamBase *block) {
	BlockStreamBase *empty_block=empty_block_list_.front();
	empty_block_list_.pop_front();
	empty_block->copyBlock(block->getBlock(),block->getsize());
	partitioned_block_list_[index].push_back(empty_block);
	return true;
}

bool PartitionedShuffleBuffer::get_one(BlockStreamBase *&block, unsigned &index){
	if(partitioned_block_list_[index].empty())
		return false;
	block=partitioned_block_list_[index].front();
	partitioned_block_list_[index].pop_front();
	return true;
}

int PartitionedShuffleBuffer::get_acturally_size(){
	int ret=0;
	for(unsigned i=0;i<n_partitions_;i++){
		ret+=partitioned_block_list_[i].size();
	}
	return ret;
}
