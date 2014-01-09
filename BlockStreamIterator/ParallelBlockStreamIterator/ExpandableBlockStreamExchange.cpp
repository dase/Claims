/*
 * ExpandableBlockStreamExchange.cpp
 *
 *  Created on: Aug 29, 2013
 *      Author: wangli
 */

#include "ExpandableBlockStreamExchange.h"
#include "ExpandableBlockStreamExchangeLower.h"
#include <libconfig.h++>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <assert.h>

#include "../../Block/BlockReadableFix.h"
#include "../../Logging.h"
#include "../../PortManager.h"
#include "../../Environment.h"
#include "../../Executor/ExchangeTracker.h"

#include "../../configure.h"
#include "../../rename.h"
ExpandableBlockStreamExchange::ExpandableBlockStreamExchange(State state)
:state(state){
	sem_open_.set_value(1);
}
ExpandableBlockStreamExchange::ExpandableBlockStreamExchange(){
	sem_open_.set_value(1);
}
ExpandableBlockStreamExchange::~ExpandableBlockStreamExchange() {
	// TODO Auto-generated destructor stub
}

bool ExpandableBlockStreamExchange::open(const PartitionOffset& partition_off){
	if(sem_open_.try_wait()){
		nexhausted_lowers=0;
		nlowers=state.lower_ip_list.size();

		for(unsigned i=0;i<nlowers;i++){
			received_block[i]=0;
		}

		socket_fd_lower_list=new int[nlowers];
//		lower_ip_array=new std::string[nlowers];

		buffer=new BlockStreamBuffer(state.block_size,nlowers*10,state.schema);
		received_block_stream_=BlockStreamBase::createBlock(state.schema,state.block_size);
		block_for_socket_=new BlockReadableFix(received_block_stream_->getSerializedBlockSize(),state.schema);
		if(PrepareTheSocket()==false)
			return false;

		if(RegisterExchange()==false){
			Logging_ExchangeIteratorEager("Register Exchange with ID=%l fails!",state.exchange_id);
		}

		if(isMaster()){
			Logging_ExchangeIteratorEager("This exchange is the master one, serialize the iterator subtree to the children...");
			if(SerializeAndSendToMulti()==false)
				return false;
		}


		if(WaitForConnectionFromLowerExchanges()==false){
			return false;
		}

		if(CreateReceiverThread()==false){
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

bool ExpandableBlockStreamExchange::next(BlockStreamBase* block){
	while(nexhausted_lowers<nlowers){
		if(buffer->getBlock(*block)){
			return true;
		}
	}
	/* all the lowers exchange are exhausted.*/
	return buffer->getBlock(*block);

}

bool ExpandableBlockStreamExchange::close(){
	CancelReceiverThread();

	for(unsigned i=0;i<nlowers;i++){
		FileClose(this->socket_fd_lower_list[i]);
	}

	block_for_socket_->~BlockReadable();
	received_block_stream_->~BlockStreamBase();
	buffer->~BlockStreamBuffer();

	delete[] socket_fd_lower_list;

	CloseTheSocket();

	Environment::getInstance()->getExchangeTracker()->LogoutExchange(ExchangeID(state.exchange_id,0));

	return true;
}


bool ExpandableBlockStreamExchange::PrepareTheSocket()
{
	struct sockaddr_in my_addr;

	//sock_fd is the socket of this node
	if((sock_fd=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("socket creation error!\n");
		return false;
	}
	my_addr.sin_family=AF_INET;

	/* apply for the port dynamicaly.*/
	if((socket_port=PortManager::getInstance()->applyPort())==0){
		Logging_ExchangeIteratorEager("Fails to apply a port for the socket. Reason: the PortManager is exhausted!");
	}
	Logging_ExchangeIteratorEager("The applied port for socket is %d",socket_port);

	my_addr.sin_port=htons(socket_port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);

	/* Enable address reuse */
	int on=1;
	setsockopt(sock_fd, SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));
//	int size=1024*1024*64;
//	setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,(const char*)&size,sizeof(int));



	if(bind(sock_fd,(struct sockaddr *)&my_addr, sizeof(struct sockaddr))==-1)
	{
		perror("bind errors!\n");
		return false;
	}

	if(listen(sock_fd, nlowers)==-1)
	{
		perror("listen errors!\n");
		return false;
	}

	Logging_ExchangeIteratorEager("socket created as: %s:%d",inet_ntoa(my_addr.sin_addr),socket_port);

	return true;
}

void ExpandableBlockStreamExchange::CloseTheSocket(){
	/* colse the sockets of the lowers*/
	for(unsigned i=0;i<nlowers;i++){
		FileClose(socket_fd_lower_list[i]);
	}

	/* close the socket of this exchange*/
	FileClose(sock_fd);

	/* return the applied port to the port manager*/
	PortManager::getInstance()->returnPort(socket_port);
}

bool ExpandableBlockStreamExchange::RegisterExchange(){
	ExchangeTracker* et=Environment::getInstance()->getExchangeTracker();
	std::ostringstream port_str;
	port_str<<socket_port;
	return et->RegisterExchange(ExchangeID(state.exchange_id,0),port_str.str());
}

bool ExpandableBlockStreamExchange::isMaster(){
	Logging_ExchangeIteratorEager("master ip=%s, self ip=%s",state.upper_ip_list[0].c_str(),Environment::getInstance()->getIp().c_str());
	return Environment::getInstance()->getIp()==state.upper_ip_list[0];
}
bool ExpandableBlockStreamExchange::SerializeAndSendToMulti(){
	IteratorExecutorMaster* IEM=IteratorExecutorMaster::getInstance();
	ExpandableBlockStreamExchangeLower::State EIELstate(state.schema,state.child,state.upper_ip_list,state.block_size,state.exchange_id);
	BlockStreamIteratorBase *EIEL=new ExpandableBlockStreamExchangeLower(EIELstate);

	if(IEM->ExecuteBlockStreamIteratorsOnSites(EIEL,state.lower_ip_list)==false){
		printf("Cannot send the serialized iterator tree to the remote node!\n");
		return false;
	}
	return true;
}

bool ExpandableBlockStreamExchange::WaitForConnectionFromLowerExchanges(){
	//wait for the lower nodes send the connection information
	socklen_t sin_size=sizeof(struct sockaddr_in);
	struct sockaddr_in remote_addr;
	unsigned count=0;
	while(1){
		if(count>=nlowers){
			return true;
		}
		Logging_ExchangeIteratorEager("Waiting for the socket connection from the lower exchange..");
		if((socket_fd_lower_list[count]=accept(sock_fd,(struct sockaddr*)&remote_addr,&sin_size))!=-1)
		{
			lower_ip_array.push_back(inet_ntoa(remote_addr.sin_addr));
			Logging_ExchangeIteratorEager("The lower exchange <%s> is connected to the socket.",lower_ip_array[count].c_str());
			count++;
		}
	}

	return true;
}

bool ExpandableBlockStreamExchange::CreateReceiverThread(){
	int error;
	error=pthread_create(&receiver_tid,NULL,receiver,this);
	if(error!=0){
		Logging_ExchangeIteratorEager("Failed to create receiver thread.");
		return false;
	}

	pthread_create(&debug_tid,NULL,debug,this);
	return true;
}
void ExpandableBlockStreamExchange::CancelReceiverThread(){
	pthread_cancel(receiver_tid);
	pthread_cancel(debug_tid);
}

void* ExpandableBlockStreamExchange::receiver(void* arg){
	ExpandableBlockStreamExchange* Pthis=(ExpandableBlockStreamExchange*)arg;

	unsigned subset=Pthis->nlowers;

	while(true){


		int old_seed=0;

		fd_set socketfds;
		FD_ZERO(&socketfds);
		int sock_fd_max=0;
		for(int i=0;i<Pthis->nlowers;i++){
			FD_SET(Pthis->socket_fd_lower_list[i],&socketfds);
			if(Pthis->socket_fd_lower_list[i]>sock_fd_max)
				sock_fd_max=Pthis->socket_fd_lower_list[i];
		}
//		for(int i=0,seed=old_seed;i<subset;i++){
//			FD_SET(Pthis->socket_fd_lower_list[seed],&socketfds);
//			if(Pthis->socket_fd_lower_list[seed]>sock_fd_max)
//				sock_fd_max=Pthis->socket_fd_lower_list[i];
//			seed=(seed+1)%Pthis->nlowers;
//		}
		/**
		 * After the select() returned, there is a loop to check which socket fd has newly coming message.
		 * The check should be in random order rather than in the increasing order, otherwise, the socket fd
		 * which is in the smaller index has larger chance to be checked than other socket fds.
		 */
//		unsigned seed=rand()%Pthis->nlowers;
		unsigned seed=old_seed;

		if(select(sock_fd_max+1,&socketfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval*)NULL)>0){

			for(unsigned j=0;j<subset;j++){
//			for(unsigned j=0;j<Pthis->nlowers;j++){
//				printf("Check [%s]",Pthis->lower_ip_array[seed].c_str());
				if(FD_ISSET(Pthis->socket_fd_lower_list[seed],&socketfds)){

					int recvtotal=0;

					while(recvtotal<Pthis->block_for_socket_->getsize()){
						int recvbytes;
						if((recvbytes=recv(Pthis->socket_fd_lower_list[seed],(char*)Pthis->block_for_socket_->getBlock()+recvtotal,Pthis->block_for_socket_->getsize()-recvtotal,0))==-1){
												perror("recv error!\n");
												return false;
						}
						recvtotal+=recvbytes;
					}

					Logging_ExchangeIteratorEager("The %d-th block is received from Lower[%s]",Pthis->received_block[seed],Pthis->lower_ip_array[seed].c_str());
					Pthis->received_block[seed]++;

					Pthis->received_block_stream_->deserialize(Pthis->block_for_socket_);
					const bool isLastBlock=Pthis->block_for_socket_->IsLastBlock();

					Pthis->buffer->insertBlock(Pthis->received_block_stream_);


					if(isLastBlock){
						Logging_ExchangeIteratorEager("*****This block is the last one.");
						Pthis->nexhausted_lowers++;
						Logging_ExchangeIteratorEager("<<<<<<<<<<<<<<<<nexhausted_lowers=%d>>>>>>>>>>>>>>>>",Pthis->nexhausted_lowers);
						Pthis->SendBlockAllConsumedNotification(Pthis->socket_fd_lower_list[seed]);
						Logging_ExchangeIteratorEager("This notification (all the blocks in the socket buffer are consumed) is send to the lower[%s].",Pthis->lower_ip_array[seed].c_str());

					}
				break;
				}
				seed=(seed+1)%Pthis->nlowers;
			}
		}

	}
}

void ExpandableBlockStreamExchange::SendBlockBufferedNotification(int target_socket_fd){
	char content='c';
	if(send(target_socket_fd,&content,sizeof(char),0)==-1){
		perror("Send error!\n");
		return ;
	}

}
void ExpandableBlockStreamExchange::SendBlockAllConsumedNotification(int target_socket_fd){
	char content='e';
	if(send(target_socket_fd,&content,sizeof(char),MSG_WAITALL)==-1){
		perror("Send error!\n");
		return ;
	}
}

void* ExpandableBlockStreamExchange::debug(void* arg){
	ExpandableBlockStreamExchange* Pthis=(ExpandableBlockStreamExchange*)arg;
	while(true){
		printf("Upper: %d blocks in buffer.\n",Pthis->buffer->getBlockInBuffer());
		usleep(100000);
	}
}
