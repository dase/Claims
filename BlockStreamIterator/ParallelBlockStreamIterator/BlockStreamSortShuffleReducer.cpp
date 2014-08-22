/*
 * BlockStreamSortShuffleReducer.cpp
 *
 *  Created on: 2014-7-24
 *      Author: casa
 */

#include "BlockStreamSortShuffleReducer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../Executor/IteratorExecutorMaster.h"
#include "BlockStreamSortShuffleMapper.h"
#include <arpa/inet.h>
#include <sys/epoll.h>

BlockStreamSortShuffleReducer::BlockStreamSortShuffleReducer() {

}

BlockStreamSortShuffleReducer::BlockStreamSortShuffleReducer(State state)
:state_(state){

}

BlockStreamSortShuffleReducer::~BlockStreamSortShuffleReducer() {

}

bool BlockStreamSortShuffleReducer::open(const PartitionOffset& part_off){
	/* prepare the socket to connect to map side, return a socket fd */
	/*
	 * 1, AF_INET is the ipv4
	 * 2, SOCK_STREAM is the type
	 * 3, protocol usually is 0
	 * get a file descriptor from a socket function.
	 * */
	int socketfd;
	if((socketfd=socket(AF_INET,SOCK_STREAM,0))==-1)
		cout<<"socket created error!"<<endl;

	/* bind a socket to a address,first we can get the address. */
	struct sockaddr_in my_addr;/* set a address */
	my_addr.sin_family=AF_INET;
	//todo: send the port to mapper side.
	int port=12032;/* here we need port to identify a process */
	my_addr.sin_port=htons(port);//why here we need to transfer the port to sin_port
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);

	if(bind(socketfd,(struct sockaddr *)&my_addr, sizeof(struct sockaddr))==-1)
		cout<<"error in bind!"<<endl;

	if(listen(socketfd,3)==-1)
		cout<<"listen error!"<<endl;

	/* here we can send the serialized stage to the mapper node,
	 * we only use one node to send the serialized task.
	 * */
	int lowers=state_.lower_ip_list_.size();
	if(part_off==0){
		/* here we use the singliton oeject, because we can avoid
		 * renewing a instance by many arguments.
		 *  */
		IteratorExecutorMaster *master=IteratorExecutorMaster::getInstance();
		BlockStreamSortShuffleMapper::State
		mstate(state_.schema_,state_.child_,state_.exchange_id_,state_.upper_ip_list_,state_.block_size_);
		for(unsigned i=0;i<lowers;i++){
			mstate.partition_off_=i;
			BlockStreamIteratorBase *m=new BlockStreamSortShuffleMapper(mstate);
			if(master->ExecuteBlockStreamIteratorsOnSite(m,state_.lower_ip_list_[i])==false);
				cout<<"send a message to slave."<<state_.lower_ip_list_[i]<<endl;
			m->~BlockStreamIteratorBase();
		}
	}

	socklen_t sin_size=sizeof(struct sockaddr_in);
	struct sockaddr_in remote_addr;
	/*here we wait for connecting from the map node*/
	cout<<"I am accepting! before"<<endl;

	int coming_count=0;
	int *mapping_fd=new int[lowers];
	while(coming_count<lowers){
		if((mapping_fd[coming_count++]=accept(socketfd,(struct sockaddr*)&remote_addr,&sin_size))!=-1)
			cout<<"I get the connection from the ip: "<<inet_ntoa(remote_addr.sin_addr)<<endl;
		else
			cout<<"error in accepting!"<<endl;
	}
	cout<<"I am accepting! after"<<endl;

	/* pthread_create a receiver to get data. */
	pthread_t receiver_tid;
	if(pthread_create(&receiver_tid,0,receiver,this)!=0)
		cout<<"create a receiver thread error!"<<endl;

	//todo: 100 must be defined.
	partitioned_buffer_=new PartitionedShuffleBuffer(state_.schema_,state_.block_size_,100,state_.lower_ip_list_.size());
	/* use the epoll to get the mapper node connections. merge them and
	 * let it be a new source by using epoll syscall to get the socket ios.
	 * */
	struct epoll_event event;
	struct epoll_event *events;
	int epfd;
	int status;
	int event_count;
	if((epfd=epoll_create(lowers))==-1)
		cout<<"create epoll file descriptor failed."<<endl;
	//here calloc= malloc+memset.
	events=(epoll_event*)calloc(lowers,sizeof(struct epoll_event));
	//register the event into the epoll kernel.
	for(unsigned i=0;i<lowers;i++){
		event.events=EPOLLIN | EPOLLET;
		event.data.fd=mapping_fd[i];
		if((status=epoll_ctl(epfd,EPOLL_CTL_ADD,mapping_fd[i],&event))==-1){
			cout<<"epoll control failed!: "<<lowers<<": "<<i<<endl;
		}
	}

	int count_=0;
	BlockStreamBase *block12=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	while(true){
		event_count=epoll_wait(epfd,events,lowers,-1);
		if(event_count==0){
			cout<<"hello? hehe![][][][][[[[][][][][][][]]["<<endl;
			break;
		}
		cout<<"get the event count!: "<<event_count<<"="<<events[0].data.fd<<endl;
		for(unsigned i=0;i<event_count;i++){
			if(events[i].events & EPOLLIN){
				/* we use read() function.*/
				int recv_bytes;
//				if((recv_bytes=read(events[i].data.fd,block->getBlock(),block->getsize()))!=-1)
				if((recv_bytes=recv(events[i].data.fd,block12->getBlock(),block12->getsize(),MSG_WAITALL ))!=-1){
					cout<<"I will recieve the data from the lower nodes."<<recv_bytes<<"block12->getsize(): "<<block12->getsize()<<endl;
					partitioned_buffer_->insert(i,block12);
					cout<<"partitioned buffer has "<<partitioned_buffer_->get_acturally_size()<<" blocks"<<endl;;
				}
			}
		}
		continue;
	}
	cout<<"++++++++++++++++++++++++++++++++++++++++"<<endl;
	partitioned_buffer_->debug();
//	BlockStreamBase::BlockStreamTraverseIterator *itr=block12->createIterator();
//	void *tuple12;
//	cout<<"I am going to display the tuples out!"<<endl;
//	while((tuple12=itr->nextTuple())!=0){
//		state_.schema_->displayTuple(tuple12,"|");
//		cout<<"count: "<<count_++<<endl;
//	}
	getchar();
	return true;
}

bool BlockStreamSortShuffleReducer::next(BlockStreamBase *block){
	return true;
}

bool BlockStreamSortShuffleReducer::close(){
	return true;
}

void* BlockStreamSortShuffleReducer::receiver(void *args){
	cout<<"in the receiver thread!"<<endl;
	return 0;
}
