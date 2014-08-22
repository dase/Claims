/*
 * BlockStreamSortShuffleMapper.cpp
 *
 *  Created on: 2014-7-24
 *      Author: casa
 */

#include "BlockStreamSortShuffleMapper.h"
#include <sys/socket.h>
#include <netdb.h>

#define DEBUG

BlockStreamSortShuffleMapper::BlockStreamSortShuffleMapper() {

}

BlockStreamSortShuffleMapper::BlockStreamSortShuffleMapper(State state)
:state_(state){

}

BlockStreamSortShuffleMapper::~BlockStreamSortShuffleMapper() {

}

bool BlockStreamSortShuffleMapper::open(const PartitionOffset& part_off){
	cout<<"in the open!"<<endl;
	state_.child_->open(part_off);
	int uppers=state_.upper_ip_list_.size();
	socketfd=new int[uppers];
	/* in this iterator, we first connect to the reducer nodes. */
	/* also, we can get the socket file descriptor*/
	for(unsigned i=0;i<state_.upper_ip_list_.size();i++){
		socketfd[i]=socket(AF_INET,SOCK_STREAM,0);

		struct hostent* host;
		string ip=state_.upper_ip_list_[part_off];
		if((host=gethostbyname(ip.c_str()))==0){
			cout<<"gethostbyname error!"<<endl;
			return false;
		}

#ifdef DEBUG
	cout<<"in mapper, I have a define a socket."<<state_.upper_ip_list_[i].c_str()<<endl;
#endif
		struct sockaddr_in serv_add;
		serv_add.sin_family=AF_INET;
		serv_add.sin_port=htons(12032);
		serv_add.sin_addr=*((struct in_addr *)host->h_addr);
		bzero(&(serv_add.sin_zero),8);

		cout<<"I am connecting the reducer!"<<endl;
		int returnvalue;
		if((returnvalue=connect(socketfd[i],(struct sockaddr *)&serv_add,sizeof(struct sockaddr)))==-1)
			cout<<"connection error!"<<endl;
		else
			cout<<"yes, connected!"<<endl;
		cout<<"finished connecting!"<<endl;
	}

	/* pthread_create a sender to get data. */
	pthread_t sender_id;
	if(pthread_create(&sender_id,0,sender,this)!=0)
		cout<<"create a sender thread error!"<<endl;

	//todo: decide 100 be replaced by memory capacity.
	shuffle_buffer_=new
			PartitionedShuffleBuffer(state_.schema_,state_.block_size_,100,state_.upper_ip_list_.size());

	/*allocate a block to buffer the data into partitioned buffer.*/
	BlockStreamBase** cur_block=new BlockStreamBase *[uppers];
	for(unsigned i=0;i<uppers;i++){
		cur_block[i]=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	}
	BlockStreamBase *block=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	/* now we can dispatch the child next data.*/
	void *tuple=0;
	void *addr=0;
	while(state_.child_->next(block)){
		BlockStreamBase::BlockStreamTraverseIterator *itr=block->createIterator();
		while((tuple=itr->nextTuple())!=0){
			int partition_id=(*(int *)tuple)%1;
			if((addr=cur_block[partition_id]->allocateTuple(state_.schema_->getTupleMaxSize()))!=0){
				state_.schema_->copyTuple(tuple,addr);
			}
			else{
				shuffle_buffer_->insert(partition_id,cur_block[partition_id]);
				cur_block[partition_id]->setEmpty();
			}
		}
		block->setEmpty();
	}
	for(unsigned i=0;i<state_.upper_ip_list_.size();i++){
		if(!cur_block[i]->Empty())
			shuffle_buffer_->insert(i,cur_block[i]);
	}

	shuffle_buffer_->externalSort();
//	shuffle_buffer->debug();
	return true;
}

bool BlockStreamSortShuffleMapper::next(BlockStreamBase *){
	/* in next, we can send the block into another node. */
	BlockStreamBase *block=
			BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	unsigned partition_id=0;
	while(shuffle_buffer_->get_one(block,partition_id)){
//		BlockStreamBase::BlockStreamTraverseIterator *itr=block->createIterator();
//		void *tuple;
//		while((tuple=itr->nextTuple())!=0){
//			state_.schema_->displayTuple(tuple,"|");
//		}
//		int total=0;
//		while(total<block->getsize()){
		int send_bytes;
		if((send_bytes=send(socketfd[partition_id],block->getBlock(),block->getsize(),MSG_WAITALL))!=-1)
			cout<<"sending a block ok!: "<<send_bytes<<endl;
		else
			break;
//		}
	}
	cout<<"////////////////////////////////"<<endl;
	getchar();
	return false;
}

bool BlockStreamSortShuffleMapper::close(){
	return true;
}

void* BlockStreamSortShuffleMapper::sender(void *args) {
	cout<<"in the sender thread!"<<endl;
	return 0;
}
