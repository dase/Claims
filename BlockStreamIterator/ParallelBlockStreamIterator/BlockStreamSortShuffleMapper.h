/*
 * BlockStreamSortShuffleMapper.h
 *
 *  Created on: 2014-7-24
 *      Author: casa
 */

#ifndef BLOCKSTREAMSORTSHUFFLEMAPPER_H_
#define BLOCKSTREAMSORTSHUFFLEMAPPER_H_

#include "../../common/Block/PartitionedShuffleBuffer.h"
#include "../../common/ids.h"
#include "../BlockStreamIteratorBase.h"

#include <vector>
#include <string>
using namespace std;

/*
 * mapper is the same as the lower of the exchange!
 * */
class BlockStreamSortShuffleMapper:public BlockStreamIteratorBase{
//	: public BlockStreamExchangeLowerBase{
public:
	struct State{
		Schema *schema_;
		BlockStreamIteratorBase *child_;
		unsigned long long int exchange_id_;
		vector<string> upper_ip_list_;
		unsigned block_size_;
		PartitionOffset partition_off_;
		State(Schema *schema,
				BlockStreamIteratorBase *child,
				unsigned long long int exchange_id,
				vector<string> upper_ip_list,
				unsigned block_size,
				PartitionOffset partition_off=0)
		:schema_(schema),
		 child_(child),
		 exchange_id_(exchange_id),
		 upper_ip_list_(upper_ip_list),
		 block_size_(block_size),
		 partition_off_(partition_off)
		{}

		State(){};

		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & schema_ & child_ & exchange_id_ & upper_ip_list_ &block_size_&partition_off_;
		}
	};

	BlockStreamSortShuffleMapper(State state);
	BlockStreamSortShuffleMapper();
	virtual ~BlockStreamSortShuffleMapper();

public:
	bool open(const PartitionOffset& part_off=0);
	bool next(BlockStreamBase*);
	bool close();

	static void *sender(void *);

private:
	State state_;
	PartitionedShuffleBuffer * shuffle_buffer_;
	int* socketfd;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_;
	}
};

#endif /* BLOCKSTREAMSORTSHUFFLEMAPPER_H_ */
