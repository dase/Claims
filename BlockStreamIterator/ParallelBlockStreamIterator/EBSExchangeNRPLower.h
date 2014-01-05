/*
 * EBSExchangeNRPLower.h
 *
 *  Created on: 2013-12-18
 *      Author: casa
 */

#ifndef EBSEXCHANGENRPLOWER_H_
#define EBSEXCHANGENRPLOWER_H_

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include "../BlockStreamIteratorBase.h"
#include "../../Block/BlockStreamBuffer.h"
#include "../../Block/PartitionedBlockBuffer.h"
#include "../../Executor/IteratorExecutorMaster.h"
#include "../../utils/Logging.h"

class EBSExchangeNRPLower:public BlockStreamIteratorBase {
public:
	struct State{
		Schema* schema;
		BlockStreamIteratorBase* child;
		unsigned long long int exchange_id;
		//Currently, support give the ip vector.
		//TODO: support ip vector provided by scheduler
		std::vector<std::string> upper_ip_list;
		unsigned block_size;
		PartitionOffset partition_offset;
		State(Schema *schema, BlockStreamIteratorBase* child, std::vector<std::string> upper_ip_list, unsigned block_size,
						unsigned long long int exchange_id=0)
		:schema(schema),child(child),upper_ip_list(upper_ip_list),block_size(block_size),exchange_id(exchange_id),partition_offset(0)
		{}
		State(){};
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & schema & child & exchange_id & upper_ip_list &block_size&partition_offset;
		}
	};

	EBSExchangeNRPLower(State state);
	EBSExchangeNRPLower();
	virtual ~EBSExchangeNRPLower();

	bool open(const PartitionOffset& partition_offset=0);
	bool next(BlockStreamBase* );
	bool close();

	static void* sender(void* arg);

	unsigned hash(void*);
	bool ConnectToUpperExchangeWithMulti(int &sock_fd,struct hostent* host,int port);
	void WaitingForNotification(int target_socket_fd);
	void WaitingForCloseNotification();

private:
	State state_;
	unsigned nuppers_;
	int socket_fd_;
	BlockStreamBuffer *buffer_;
	BlockStreamBase* block_stream_for_asking_;
	BlockStreamBase* block_for_sending_;
	pthread_t sender_tid_;
	Logging *logging_;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
			ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_;
	}
};

#endif /* EBSEXCHANGENRPLOWER_H_ */
