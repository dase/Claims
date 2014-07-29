/*
 * ExpandableBlockStreamExchangeMaterialized.cpp
 *
 *  Created on: Sep 4, 2013
 *      Author: wangli
 */
#include <pthread.h>
#include <sys/socket.h>
#include "ExpandableBlockStreamExchangeLowerMaterialized.h"
#include "ExpandableBlockStreamExchangeMaterialized.h"
#include "../../common/Logging.h"
#include "../../Environment.h"

ExpandableBlockStreamExchangeMaterialized::ExpandableBlockStreamExchangeMaterialized(State state)
:BlockStreamExchangeBase(state.upper_ip_list_,state.lower_ip_list_.size()),state_(state){
	sem_open_.set_value(1);

}

ExpandableBlockStreamExchangeMaterialized::~ExpandableBlockStreamExchangeMaterialized() {

}

bool ExpandableBlockStreamExchangeMaterialized::open(const PartitionOffset& part_off){
	if(sem_open_.try_wait()){
		nexhausted_lowers_=0;
		received_block_stream_=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
		block_for_socket_=new Block(received_block_stream_->getSerializedBlockSize());
		buffer=new BlockStreamBuffer(state_.block_size_,nlowers_*10,state_.schema_);
		/*
		 * we persist the sorted block in disk and send them to reducer side(upper).
		 * not perfect that we must collect all tuples before we sort the tuple,
		 * so we use a buffer to collect every tuple and sort, then we insert every
		 * block size into buffer one by one to trigger the semaphere.
		 * */
		sorted_buffer_=new SortedBlockStreamBuffer(nlowers_,state_.schema_,state_.block_size_,state_.orderbyKey_);
		sorted_buffer_->init();
		if(PrepareTheSocket()==false)
			return false;
		if(RegisterExchange(ExchangeID(state_.exchange_id_,0))==false){
			Logging_ExpandableBlockStreamExchangeMaterialized("Register Exchange with ID=%l fails!",state_.exchange_id_);
		}
		if(isMaster()){
			Logging_ExpandableBlockStreamExchangeMaterialized("This exchange is the master one, serialize the iterator subtree to the children...");
			if(SerializeAndSendToMulti()==false)
				return false;
		}
		if(WaitForConnectionFromLowerExchanges()==false){
			return false;
		}
		if(pthread_create(&receiver_tid,NULL,receiver,this)!=0){
			Logging_ExpandableBlockStreamExchangeMaterialized("Failed to create receiver thread.");
			return false;
		}
		open_finished_=true;
		return true;
	}
	else{
		while(!open_finished_){
			usleep(1);
		}
		return true;
	}
}

bool ExpandableBlockStreamExchangeMaterialized::next(BlockStreamBase* block){
	while(flags_==0){
		if(buffer->getBlock(*block)){
			return true;
		}
	}
	/* all the lowers exchange are exhausted.*/
	Logging_ExpandableBlockStreamExchangeMaterialized("all the child is exhausted in the next!\n");
	return buffer->getBlock(*block);
}

bool ExpandableBlockStreamExchangeMaterialized::close(){
	FILE* file;
	void* addr;
	sem_open_.set_value(1);
	block_for_socket_->~Block();
	received_block_stream_->~BlockStreamBase();
	buffer->~BlockStreamBuffer();
	CloseTheSocket();
	Environment::getInstance()->getExchangeTracker()->LogoutExchange(ExchangeID(state_.exchange_id_,0));
	return true;
}

bool ExpandableBlockStreamExchangeMaterialized::SerializeAndSendToMulti(){
	IteratorExecutorMaster* IEM=IteratorExecutorMaster::getInstance();
	ExpandableBlockStreamExchangeLowerMaterialized::State EIELstate(state_.schema_,state_.child_,state_.upper_ip_list_,state_.block_size_,state_.exchange_id_);
	for(unsigned i=0;i<state_.lower_ip_list_.size();i++){
		/* set the partition offset*/
		EIELstate.partition_off_=i;
		BlockStreamIteratorBase *EIEL=new ExpandableBlockStreamExchangeLowerMaterialized(EIELstate);
		if(IEM->ExecuteBlockStreamIteratorsOnSite(EIEL,state_.lower_ip_list_[i])==false){
			printf("Cannot send the serialized iterator tree to the remote node!\n");
			return false;
		}
		EIEL->~BlockStreamIteratorBase();
	}
	return true;
}

void* ExpandableBlockStreamExchangeMaterialized::receiver(void* arg){
	ExpandableBlockStreamExchangeMaterialized* Pthis=(ExpandableBlockStreamExchangeMaterialized*)arg;
	unsigned subset=Pthis->nlowers_;
	while(true){
		int old_seed=0;
		fd_set socketfds;
		FD_ZERO(&socketfds);
		int sock_fd_max=0;
		for(int i=0;i<Pthis->nlowers_;i++){
			FD_SET(Pthis->socket_fd_lower_list_[i],&socketfds);
			if(Pthis->socket_fd_lower_list_[i]>sock_fd_max)
				sock_fd_max=Pthis->socket_fd_lower_list_[i];
		}
		/**
		 * After the select() returned, there is a loop to check which socket fd has newly coming message.
		 * The check should be in random order rather than in the increasing order, otherwise, the socket fd
		 * which is in the smaller index has larger chance to be checked than other socket fds.
		 */
		int seed=old_seed;
		if(select(sock_fd_max+1,&socketfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL)>0){
			for(unsigned j=0;j<subset;j++){
				if(FD_ISSET(Pthis->socket_fd_lower_list_[seed],&socketfds)){
					int recvtotal=0;
					while(recvtotal<Pthis->block_for_socket_->getsize()){
						int recvbytes;
						if((recvbytes=recv(Pthis->socket_fd_lower_list_[seed],(char*)Pthis->block_for_socket_->getBlock()+recvtotal,Pthis->block_for_socket_->getsize()-recvtotal,MSG_WAITALL))==-1){
							perror("recv error!\n");
							return false;
						}
						recvtotal+=recvbytes;
					}
					Pthis->received_block_stream_->deserialize(Pthis->block_for_socket_);
					const bool isLastBlock=	Pthis->received_block_stream_->Empty();
//					Pthis->buffer->insertBlock(Pthis->received_block_stream_);
					if(!isLastBlock)
						Pthis->sorted_buffer_->insertIntoSortedLists(Pthis->received_block_stream_,seed);
					if(isLastBlock){
						Logging_ExpandableBlockStreamExchangeMaterialized("*****This block is the last one.");
						Pthis->nexhausted_lowers_++;
						Logging_ExpandableBlockStreamExchangeMaterialized("<<<<<<<<<<<<<<<<nexhausted_lowers=%d>>>>>>>>>>>>>>>>",Pthis->nexhausted_lowers_);
						Pthis->SendBlockAllConsumedNotification(Pthis->socket_fd_lower_list_[seed]);
						Logging_ExpandableBlockStreamExchangeMaterialized("This notification (all the blocks in the socket buffer are consumed) is send to the lower[%s].",Pthis->lower_ip_array_[seed].c_str());
						if(Pthis->nexhausted_lowers_==Pthis->nlowers_){
							Pthis->sorted_buffer_->sortAllTuples();
							/* construct a block and insert into the buffer */
							Pthis->sorted_buffer_->insertIntoBuffer(Pthis->buffer);
							Logging_ExpandableBlockStreamExchangeMaterialized("finished global sorting!");
							Pthis->flags_=1;
						}
					}
					break;
				}
				seed=(seed+1)%Pthis->nlowers_;
			}
		}
	}
}
