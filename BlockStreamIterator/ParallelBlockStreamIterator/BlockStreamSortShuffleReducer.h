/*
 * BlockStreamSortShuffleReducer.h
 *
 *  Created on: 2014-7-24
 *      Author: casa
 */

#ifndef BLOCKSTREAMSORTSHUFFLEREDUCER_H_
#define BLOCKSTREAMSORTSHUFFLEREDUCER_H_

#include "../BlockStreamIteratorBase.h"
#include "../../common/Schema/Schema.h"
#include "../../common/Block/Block.h"
#include "../../common/Block/BlockStream.h"
#include "../../common/Block/BlockStreamBuffer.h"
#include "../../common/Block/PartitionedShuffleBuffer.h"
#include "../../utility/lock.h"

#include <sys/types.h>

/*
 * reducer is the same as the upper of the exchange!
 * */
class BlockStreamSortShuffleReducer:public BlockStreamIteratorBase{
//	: public BlockStreamExchangeBase{
public:
	struct State{
		Schema *schema_;/* the shuffleReducer handle the schema */
		BlockStreamIteratorBase *child_;/* the child of the shuffleReducer: shuffleMapper */
		vector<string> lower_ip_list_;
		vector<string> upper_ip_list_;
		unsigned long long int exchange_id_;
		unsigned block_size_;
		vector<unsigned> orderKeys_;
		State(Schema *schema,
				BlockStreamIteratorBase *child,
				vector<string> lower_ip_list,
				vector<string> upper_ip_list,
				unsigned long long int exchange_id,
				unsigned block_size,
				vector<unsigned> orderKeys)
		:schema_(schema),
		 child_(child),
		 lower_ip_list_(lower_ip_list),
		 upper_ip_list_(upper_ip_list),
		 exchange_id_(exchange_id),
		 block_size_(block_size),
		 orderKeys_(orderKeys){}
		State(){}
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & schema_ & child_ & lower_ip_list_ & upper_ip_list_ & exchange_id_ & block_size_ & orderKeys_;
		}
	};

	BlockStreamSortShuffleReducer(State state);
	BlockStreamSortShuffleReducer();
	virtual ~BlockStreamSortShuffleReducer();

	bool open(const PartitionOffset& part_off=0);
	bool next(BlockStreamBase* block);
	bool close();

private:
	static void* receiver(void* arg);

private:
	State state_;
	PartitionedShuffleBuffer *partitioned_buffer_;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_;
	}
};

#endif /* BLOCKSTREAMSORTSHUFFLEREDUCER_H_ */
