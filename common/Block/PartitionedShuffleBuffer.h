/*
 * PartitionedShuffleBuffer.h
 *
 *  Created on: 2014-8-14
 *      Author: claims
 */

#ifndef PARTITIONEDSHUFFLEBUFFER_H_
#define PARTITIONEDSHUFFLEBUFFER_H_

#include "../../common/Block/BlockStream.h"
#include "../../common/Schema/Schema.h"

#include <list>
using namespace std;

class PartitionedShuffleBuffer {
public:
	PartitionedShuffleBuffer(Schema *, unsigned, unsigned , unsigned );
	virtual ~PartitionedShuffleBuffer();

public:
	bool insert(unsigned , BlockStreamBase *);
	bool get_one(BlockStreamBase *&, unsigned &index);
	int get_acturally_size();

	void externalSort(){
		cout<<"entering sort in the buffer!"<<endl;
	};

public:
	void debug(){
		cout<<"partitioned_block_list_[0].size(): "<<partitioned_block_list_[0].size()<<endl;
		cout<<"n_partitions_: "<<n_partitions_<<endl;
		for(unsigned i=0;i<n_partitions_;i++){
			while(!partitioned_block_list_[i].empty()){
				BlockStreamBase *block=partitioned_block_list_[i].front();
				partitioned_block_list_[i].pop_front();
				BlockStreamBase::BlockStreamTraverseIterator *itr=
						block->createIterator();
				void *tuple;
				while((tuple=itr->nextTuple())!=0){
					schema_->displayTuple(tuple,"|");
					cout<<"count: "<<count++<<endl;
				}
			}
		}
	}

private:
	/*when initialization, we allocate n blocks*/
	list<BlockStreamBase *> empty_block_list_;
	list<BlockStreamBase *>* partitioned_block_list_;

	Schema *schema_;
	unsigned block_size_;
	unsigned n_blocks_;
	unsigned n_partitions_;

	/* for debug*/
	unsigned count;
};

#endif /* PARTITIONEDSHUFFLEBUFFER_H_ */
