/*
 * EBSExchangeNRPLower.cpp
 *
 *  Created on: 2013-12-18
 *      Author: casa
 */

#include "EBSExchangeNRPLower.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../../PortManager.h"
#include "../../Environment.h"


EBSExchangeNRPLower::EBSExchangeNRPLower(State state) {
	logging_=new ExchangeIteratorEagerLowerLogging();
}

EBSExchangeNRPLower::EBSExchangeNRPLower() {
	logging_=new ExchangeIteratorEagerLowerLogging();
}

EBSExchangeNRPLower::~EBSExchangeNRPLower() {

}

bool EBSExchangeNRPLower::open(const PartitionOffset& partition_offset){
	cout<<"I am in the lower open!!!"<<endl;
	block_stream_for_asking_=BlockStreamBase::createBlock(state_.schema,state_.block_size);
	block_for_sending_=BlockStreamBase::createBlock(state_.schema,state_.block_size);
	buffer_=new BlockStreamBuffer(state_.block_size,3,state_.schema);
	struct hostent *host;
	if((host=gethostbyname(state_.upper_ip_list[partition_offset].c_str()))==0){
		perror("gethostbyname errors!\n");
		return false;
	}

	if((socket_fd_=socket(AF_INET, SOCK_STREAM,0))==-1){
		perror("socket creation errors!\n");
		return false;
	}

	ExchangeTracker* et=Environment::getInstance()->getExchangeTracker();
	int upper_port;
	if((upper_port=et->AskForSocketConnectionInfo(state_.exchange_id,state_.upper_ip_list[partition_offset]))==0){
		logging_->elog("Fails to ask %s for socket connection info, the exchange id=%d",state_.upper_ip_list[partition_offset].c_str(),state_.exchange_id);
	}

	if(ConnectToUpperExchangeWithMulti(socket_fd_,host,upper_port)==false)
		return false;

	int error;
	error=pthread_create(&sender_tid_,NULL,sender,this);
	if(error!=0){
		logging_->elog("Failed to create the sender thread.");
		return false;
	}

	return true;
}

bool EBSExchangeNRPLower::next(BlockStreamBase *block){
	block_stream_for_asking_->setEmpty();
	if(state_.child->next(block_stream_for_asking_)){
		buffer_->insertBlock(block_stream_for_asking_);
	}
	else{
		// 传递一个空块来看是不是已经到最后一个了
		buffer_->insertBlock(block_stream_for_asking_);
	}
}

bool EBSExchangeNRPLower::close(){

}

void *EBSExchangeNRPLower::sender(void *arg){
	EBSExchangeNRPLower *Pthis=(EBSExchangeNRPLower*)arg;
	while(true){
		if(!Pthis->buffer_->Empty()){
			Pthis->buffer_->getBlock(*Pthis->block_for_sending_);
			int sendtotal=0;
			while(sendtotal<Pthis->block_for_sending_->getsize()){
				int recvbytes;
				if((recvbytes=send(Pthis->socket_fd_,(char*)Pthis->block_for_sending_->getBlock()+sendtotal,Pthis->block_for_sending_->getsize()-sendtotal,0))==-1){
					perror("recv error!\n");
					return false;
				}
				sendtotal+=recvbytes;
			}
		}
	}
}

bool EBSExchangeNRPLower::ConnectToUpperExchangeWithMulti(int &sock_fd,struct hostent* host,int port){
	struct sockaddr_in serv_add;
	serv_add.sin_family=AF_INET;
	serv_add.sin_port=htons(port);
	serv_add.sin_addr=*((struct in_addr*)host->h_addr);
	bzero(&(serv_add.sin_zero),8);

	int returnvalue;

	if((returnvalue=connect(sock_fd,(struct sockaddr *)&serv_add, sizeof(struct sockaddr)))==-1)
	{
		logging_->elog("Fails to connect remote socket: %s:%d",inet_ntoa(serv_add.sin_addr),port);
		return false;
	}
	logging_->log("connected to the Master socket %s:%d\n",inet_ntoa(serv_add.sin_addr),port);
	return true;
}

void EBSExchangeNRPLower::WaitingForNotification(int target_socket_fd){

}

void EBSExchangeNRPLower::WaitingForCloseNotification(){

}
