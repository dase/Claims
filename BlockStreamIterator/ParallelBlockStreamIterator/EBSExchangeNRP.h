/*
 * EBSExchangeNRP.h
 *
 *
 *  Created on: 2013-12-18
 *      Author: casa
 */

#ifndef EBSEXCHANGENRP_H_
#define EBSEXCHANGENRP_H_

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include <vector>
#include <string>
#include "../BlockStreamIteratorBase.h"
#include "../../Executor/IteratorExecutorMaster.h"
#include "../../Schema/Schema.h"
#include "../../Block/synch.h"
#include "../../Block/BlockStreamBuffer.h"
#include "../../Block/BlockStream.h"
#include "../../Block/BlockReadableFix.h"
#include "../../utils/Logging.h"
#include "../../ids.h"

class EBSExchangeNRP:public BlockStreamIteratorBase {
public:
	struct State{
		Schema *schema;
		BlockStreamIteratorBase *child;
		unsigned block_size;
		unsigned long long int exchange_id;
		std::vector<std::string> lower_ip_list;
		std::vector<std::string> upper_ip_list;
		std::vector<NodePair> node_pair_list;
		State(Schema* schema, BlockStreamIteratorBase* child,unsigned block_size,std::vector<std::string> lower_ip_list,std::vector<std::string> upper_ip_list,unsigned long long int exchange_id,std::vector<NodePair> node_pair_list)
		:schema(schema),child(child),block_size(block_size),exchange_id(exchange_id),lower_ip_list(lower_ip_list),upper_ip_list(upper_ip_list),node_pair_list(node_pair_list){}
		State(){};
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version){
			ar & schema & child  & block_size & exchange_id & lower_ip_list & upper_ip_list;// & node_pair_list;
		}
	};
	EBSExchangeNRP(State state);
	EBSExchangeNRP();
	virtual ~EBSExchangeNRP();

	bool open(const PartitionOffset& partition=0);
	bool next(BlockStreamBase* block);
	bool close();

protected:
	bool PrepareTheSocket();
	bool RegisterExchange();
	bool checkOtherUpperRegistered();
	bool isMaster();
	bool SerializeAndSendToMulti();
//	bool WaitForConnectionFromLowerExchanges();
	bool CreateReceiverThread();
	void CancelReceiverThread();
//	void SendBlockBufferedNotification(int target_socket_fd);
//	void SendBlockAllConsumedNotification(int target_socket_fd);
	void CloseTheSocket();
	bool SetSocketNonBlocking(int socket_fd);
	static void* receiver(void* arg);

private:
	State state_;
	semaphore sem_open_;
	int sock_fd;
	unsigned nlowers;
	unsigned socket_port;
	pthread_t receiver_tid;
	bool open_finished_;
	Logging *logging_;
	BlockStreamBase *receiving_block_;
	BlockStreamBuffer *buffer_;


private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & boost::serialization::base_object<BlockStreamIteratorBase>(*this) & state_;
    }

};

#endif /* EBSEXCHANGENRP_H_ */
