/*
 * EBSExchangeNRP.cpp
 *
 *  Created on: 2013-12-18
 *      Author: casa
 */

#include "EBSExchangeNRP.h"
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "EBSExchangeNRPLower.h"
#include "../../PortManager.h"
#include "../../Environment.h"
#include "../../utils/Logging.h"

EBSExchangeNRP::EBSExchangeNRP(State state)
:state_(state){
	sem_open_.set_value(1);
	open_finished_=false;
	logging_=new ExchangeIteratorEagerLogging();
}

EBSExchangeNRP::EBSExchangeNRP() {
	sem_open_.set_value(1);
	open_finished_=false;
	logging_=new ExchangeIteratorEagerLogging();
}

EBSExchangeNRP::~EBSExchangeNRP() {

}

bool EBSExchangeNRP::open(const PartitionOffset& partition_offset){
	cout<<"enter the open function!!!"<<endl;
	if(sem_open_.try_wait()){
		cout<<"I am entering the open!!!"<<endl;
		nlowers=state_.lower_ip_list.size();

		buffer_=new BlockStreamBuffer(state_.block_size,nlowers*10,state_.schema);
		receiving_block_=BlockStreamBase::createBlock(state_.schema,state_.block_size);

		if(PrepareTheSocket()==false){
			return false;
		}

		if(SetSocketNonBlocking(sock_fd)==false){
			return false;
		}

		if(RegisterExchange()==false){
			logging_->elog("Register Exchange with ID=%d fails!",state_.exchange_id);
		}

		if(isMaster()){
			cout<<"I am in the master of one level!!!"<<endl;
			/*TODO: According to a bug reported by dsc, the master exchangeupper should check whether other
			 *  uppers have registered to exchangeTracker. Otherwise, the lower may fails to connection to the
			 *  exchangeTracker of some uppers when the lower nodes receive the exchagnelower, as some uppers
			 *  have not register the exchange_id to the exchangeTracker.
			*/
			logging_->log("Synchronizing....");
			checkOtherUpperRegistered();
			logging_->log("Synchronized!");
			logging_->log("This exchange is the master one, serialize the iterator subtree to the children...");
			if(SerializeAndSendToMulti()==false)
				return false;
		}
//		else{
//			cout<<"I am in the slave of one level!!!"<<endl;
//			getchar();
//			getchar();
//			getchar();
//			getchar();
//			getchar();
//			getchar();
//
//		}


	//		if(WaitForConnectionFromLowerExchanges()==false){
	//			return false;
	//		}

		cout<<"in the EBSExchangeNRP::open!!!"<<endl;

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

bool EBSExchangeNRP::next(BlockStreamBase *block){
	cout<<"in the next of exchange upper!!!---------------------->"<<endl;
	while(true){
		if(buffer_->getBlock(*block)){
			return true;
		}
	}
	return false;
}

bool EBSExchangeNRP::close(){

}

bool EBSExchangeNRP::PrepareTheSocket(){
	struct sockaddr_in my_addr;

	//sock_fd is the socket of this node
	if((sock_fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("socket creation error!\n");
		return false;
	}
	my_addr.sin_family=AF_INET;

	/* apply for the port dynamicaly.*/
	if((socket_port=PortManager::getInstance()->applyPort())==0){
		logging_->elog("Fails to apply a port for the socket. Reason: the PortManager is exhausted!");
	}
	logging_->log("The applied port for socket is %d",socket_port);

	my_addr.sin_port=htons(socket_port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);

	/* Enable address reuse */
	int on=1;
	setsockopt(sock_fd, SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));

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

	logging_->log("socket created as: %s:%d",inet_ntoa(my_addr.sin_addr),socket_port);

	return true;
}

bool EBSExchangeNRP::RegisterExchange(){
	ExchangeTracker* et=Environment::getInstance()->getExchangeTracker();
	std::ostringstream port_str;
	port_str<<socket_port;
	return et->RegisterExchange(state_.exchange_id,port_str.str());
}

bool EBSExchangeNRP::checkOtherUpperRegistered(){
	ExchangeTracker* et=Environment::getInstance()->getExchangeTracker();
	for(unsigned i=0;i<state_.upper_ip_list.size();i++)
		cout<<"check: "<<state_.upper_ip_list[i].c_str()<<endl;
	for(unsigned i=0;i<state_.upper_ip_list.size();i++){
		cout<<"check upper ip list done!!!"<<state_.upper_ip_list[i].c_str()<<endl;
		std::string ip=state_.upper_ip_list[i];
		/* Repeatedly ask node with ip for port information untile the received port is other than 0, which means
		 * that the exchangeId on noede ip is registered to the exchangeTracker*/
		while(et->AskForSocketConnectionInfo(state_.exchange_id,ip)==0){
			usleep(1);
		}
		logging_->log("ExchangeID[%lld] is synchronized in %s",state_.exchange_id,ip.c_str());
	}
}

bool EBSExchangeNRP::isMaster(){
	logging_->log("master ip=%s, self ip=%s",state_.upper_ip_list[0].c_str(),Environment::getInstance()->getIp().c_str());
	return Environment::getInstance()->getIp()==state_.upper_ip_list[0];
}

bool EBSExchangeNRP::SerializeAndSendToMulti(){
	IteratorExecutorMaster* IEM=IteratorExecutorMaster::getInstance();
	EBSExchangeNRPLower::State EIELstate(state_.schema,state_.child,state_.upper_ip_list,state_.block_size,state_.exchange_id);

	for(unsigned i=0;i<nlowers;i++){
		EIELstate.partition_offset=i;
		BlockStreamIteratorBase *EIEL=new EBSExchangeNRPLower(EIELstate);
		cout<<"lower ip list: "<<state_.lower_ip_list[i+1]<<endl;
		if(IEM->ExecuteBlockStreamIteratorsOnSite(EIEL,state_.lower_ip_list[i+1])==false){
			logging_->elog("Cannot send the serialized iterator tree to the remote node!\n");
			return false;
		}
		EIEL->~BlockStreamIteratorBase();
	}
	return true;
}

bool EBSExchangeNRP::CreateReceiverThread(){
	int error;
	error=pthread_create(&receiver_tid,NULL,receiver,this);
	if(error!=0){
		logging_->elog("Failed to create receiver thread.");
		return false;
	}

	return true;
}

void EBSExchangeNRP::CancelReceiverThread(){

}

void EBSExchangeNRP::CloseTheSocket(){

}

bool EBSExchangeNRP::SetSocketNonBlocking(int socket_fd){
	int flags, s;

	flags = fcntl (socket_fd, F_GETFL, 0);
	if (flags == -1)
	{
	  perror ("fcntl");
	  return false;
	}

	flags |= O_NONBLOCK;
	s = fcntl (socket_fd, F_SETFL, flags);
	if (s == -1)
	{
	  perror ("fcntl");
	  return false;
	}

	return true;
}

void* EBSExchangeNRP::receiver(void* arg){
	EBSExchangeNRP *Pthis=(EBSExchangeNRP *)(arg);
	while(true){
		int recvtotal=0;
		cout<<"the block size is :"<<endl;
		cout<<"the block size is :"<<Pthis->receiving_block_->getsize()<<endl;
		while(recvtotal<Pthis->receiving_block_->getsize()){
			int recvbytes;
			if((recvbytes=recv(Pthis->sock_fd,(char*)Pthis->receiving_block_->getBlock()+recvtotal,Pthis->receiving_block_->getsize()-recvtotal,MSG_WAITALL))==-1){
				perror("recv error!\n");
				return false;
			}
			recvtotal+=recvbytes;
		}

//		const bool isLastBlock=Pthis->receiving_block_->IsLastBlock();
//		Pthis->buffer_->insertBlock(Pthis->receiving_block_);
//
//		if(isLastBlock){
//			Pthis->nlowers;
//		}

//		break;
	}
}
