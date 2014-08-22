/*
 * ExpandableBlockStreamExchangeLower.cpp
 *
 *  Created on: Aug 29, 2013
 *      Author: wangli
 */

#include "ExpandableBlockStreamExchangeLowerMaterialized.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../configure.h"
#include "../../common/rename.h"
#include "../../common/Logging.h"
#include "../../Executor/ExchangeTracker.h"
#include "../../Environment.h"
#include "../../common/Logging.h"

ExpandableBlockStreamExchangeLowerMaterialized::ExpandableBlockStreamExchangeLowerMaterialized(State state)
:state_(state),BlockStreamExchangeLowerBase(){

}

ExpandableBlockStreamExchangeLowerMaterialized::~ExpandableBlockStreamExchangeLowerMaterialized() {

}

bool ExpandableBlockStreamExchangeLowerMaterialized::open(const PartitionOffset& ){
	state_.child_->open(state_.partition_off_);
	partition_off=state_.partition_off_;
	nuppers_=state_.upper_ip_list_.size();
	child_exhausted_=false;
	socket_fd_upper_list_=new int[nuppers_];
	disk_fd_list_=new int[nuppers_];
	disk_file_length_list_=new unsigned[nuppers_];
	disk_file_cur_list_=new unsigned[nuppers_];
	block_stream_for_asking_=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	block_for_sending_=new Block(block_stream_for_asking_->getSerializedBlockSize(),state_.schema_);
	block_for_inserting_to_buffer_=new Block(block_stream_for_asking_->getSerializedBlockSize(),state_.schema_);
	cur_block_stream_list_= new BlockStreamBase*[nuppers_];
	for(unsigned i=0;i<nuppers_;i++){
		cur_block_stream_list_[i]=BlockStreamBase::createBlock(state_.schema_,state_.block_size_);
	}
	buffer_=new PartitionedBlockBuffer(nuppers_,block_stream_for_asking_->getSerializedBlockSize());
	for(unsigned upper_id=0;upper_id<state_.upper_ip_list_.size();upper_id++){
		if(!ConnectToUpper(ExchangeID(state_.exchange_id_,0),state_.upper_ip_list_[upper_id],socket_fd_upper_list_[upper_id])){
			return false;
		}
	}
	int error;
	error=pthread_create(&sender_tid_,NULL,materialize_and_send,this);
	if(error!=0){
		Logging_ExpandableBlockStreamExchangeLM("Failed to create the sender thread.");
		return false;
	}
	readsendedblocks=0;
	return true;
}

bool ExpandableBlockStreamExchangeLowerMaterialized::next(BlockStreamBase*){
	void* tuple_from_child;
	void* tuple_in_cur_block_stream;
	block_stream_for_asking_->setEmpty();
	if(state_.child_->next(block_stream_for_asking_)){
		Logging_ExpandableBlockStreamExchangeLM("->next: enter the next function\n");
		BlockStreamBase::BlockStreamTraverseIterator* traverse_iterator=block_stream_for_asking_->createIterator();
		while((tuple_from_child=traverse_iterator->nextTuple())>0){
//			cout<<"get a tuple from iterator."<<endl;
			const unsigned partition_id=hash(reinterpret_cast<char*>(tuple_from_child)+8);
			const unsigned bytes=state_.schema_->getTupleActualSize(tuple_from_child);
			while(!(tuple_in_cur_block_stream=cur_block_stream_list_[partition_id]->allocateTuple(bytes))){
				cur_block_stream_list_[partition_id]->serialize(*block_for_inserting_to_buffer_);
				cout<<"insert into the buffer!"<<endl;
				buffer_->insertBlockToPartitionedList(block_for_inserting_to_buffer_,partition_id);
				cur_block_stream_list_[partition_id]->setEmpty();
//				tuple_in_cur_block_stream=cur_block_stream_list_[partition_id]->allocateTuple(bytes);
				Logging_ExpandableBlockStreamExchangeLM("->next: a block is inserted into the buffer\n");
			}
			state_.schema_->copyTuple(tuple_from_child,tuple_in_cur_block_stream);
//			cout<<"count: "<<count++<<endl;
		}
		return true;
	}
	else{
		/* the child iterator is exhausted. We add the cur block steram block into the buffer*/
		for(unsigned i=0;i<nuppers_;i++){
			Logging_ExpandableBlockStreamExchangeLM("||||||Fold the last||||||!");
			cur_block_stream_list_[i]->serialize(*block_for_inserting_to_buffer_);
			buffer_->insertBlockToPartitionedList(block_for_inserting_to_buffer_,i);

			/* the following stupid three lines send a block with last block bytes to the buffer.
			 * TODO: making any change to the three lines would make them less stupid.
			 */
			cur_block_stream_list_[i]->setEmpty();
			cur_block_stream_list_[i]->serialize(*block_for_inserting_to_buffer_);
			buffer_->insertBlockToPartitionedList(block_for_inserting_to_buffer_,i);

		}
		/*
		 * waiting until all the block in the buffer has been transformed to the uppers.
		 */
		Logging_ExpandableBlockStreamExchangeLM("Waiting until all the blocks in the buffer is sent!");
		child_exhausted_=true;
		while(!buffer_->isEmpty()){
			usleep(1);

		}
		/*
		 * waiting until all the uppers send the close notification which means that
		 * blocks in the uppers' socket buffer have all been consumed.
		 */
		Logging_ExpandableBlockStreamExchangeLM("Waiting for close notification!");
		for(unsigned i=0;i<nuppers_;i++){
			WaitingForCloseNotification(socket_fd_upper_list_[i]);
		}
		Logging_ExpandableBlockStreamExchangeLM("....passed!");
		return false;
	}
}

unsigned ExpandableBlockStreamExchangeLowerMaterialized::hash(void* value){
	return *(int*)value%nuppers_;
}

bool ExpandableBlockStreamExchangeLowerMaterialized::close(){
	Logging_ExpandableBlockStreamExchangeLM("The sender thread is killed in the close() function!");
	pthread_cancel(sender_tid_);
	state_.child_->close();
	block_stream_for_asking_->~BlockStreamBase();
	block_for_sending_->~Block();
	block_for_inserting_to_buffer_->~Block();
	for(unsigned i=0;i<nuppers_;i++){
		cur_block_stream_list_[i]->~BlockStreamBase();
	}
	buffer_->~PartitionedBlockBuffer();
	delete [] cur_block_stream_list_;
	delete [] socket_fd_upper_list_;
	return true;
}

int ExpandableBlockStreamExchangeLowerMaterialized::getBlockFromFile(Block &block){
	unsigned seed=rand()%nuppers_;
	for(unsigned i=0;i<nuppers_;i++){
		if(disk_file_cur_list_[i]+block.getsize()<=disk_file_length_list_[i]){
			const int read_size=read(disk_fd_list_[i],block.getBlock(),block.getsize());
			assert(read_size==block.getsize());
			disk_file_cur_list_[i]+=block.getsize();
			return i;
		}
		seed=(seed=1)%nuppers_;
	}
	return -1;
}

void ExpandableBlockStreamExchangeLowerMaterialized::Send(){
	printf("in Send!\n");
	/* open the file*/
	Logging_ExpandableBlockStreamExchangeLM("open the file!\n");
	std::string temp_file_dir="/home/claims/temp/exchange/";
	for(unsigned i=0;i<nuppers_;i++){
		std::ostringstream file_name;
		file_name<<temp_file_dir<<"exchange_"<<state_.exchange_id_<<"_"<<i<<"_"<<partition_off;
		lseek(disk_fd_list_[i],0,SEEK_SET);
		disk_file_length_list_[i]=lseek(disk_fd_list_[i],0,SEEK_END);
		disk_file_cur_list_[i]=0;
		lseek(disk_fd_list_[i],0,SEEK_SET);
	}
	Logging_ExpandableBlockStreamExchangeLM("the file is opened!\n");
	while(true){
		int partition_id=getBlockFromFile(*block_for_sending_);
		if(partition_id<0){
			break;
		}
		int sendtotal=0;
		while(sendtotal<block_for_sending_->getsize()){
			int recvbytes;
			if((recvbytes=send(socket_fd_upper_list_[partition_id],(char*)block_for_sending_->getBlock()+sendtotal,block_for_sending_->getsize()-sendtotal,MSG_WAITALL))==-1){
				perror("recv error!\n");
				return ;
			}
			sendtotal+=recvbytes;
		}
		sendedblocks++;
		readsendedblocks++;
	}
	/* close the file
	 * TODO: delete the file when necessary*/
	for(unsigned i=0;i<nuppers_;i++){
		FileClose(disk_fd_list_[i]);
		}
}

bool ExpandableBlockStreamExchangeLowerMaterialized::Materialize(){
	/* create the file*/
	std::string temp_file_dir="/home/claims/temp/exchange/";
	for(unsigned i=0;i<nuppers_;i++){
		std::ostringstream file_name;
		file_name<<temp_file_dir<<"exchange_"<<state_.exchange_id_<<"_"<<i<<"_"<<partition_off;
		disk_fd_list_[i]=FileOpen(file_name.str().c_str(),O_RDWR|O_APPEND|O_TRUNC|O_CREAT,S_IWUSR|S_IRUSR);
		if(disk_fd_list_[i]==-1){
			printf("Cannot open file %s! Reason: %s\n",file_name.str().c_str(),strerror(errno));
			return false;
		}
	}
	Logging_ExpandableBlockStreamExchangeLM("The intermediate file is created!\n");
	Block* block_for_storing=new Block(block_stream_for_asking_->getSerializedBlockSize());
	while(!child_exhausted_){
		if(buffer_->getBlockInBuffer()>0){
			Logging_ExpandableBlockStreamExchangeLM("no empty in buffer!\n");
			const unsigned partition_id=buffer_->getBlock(*block_for_storing);
			const unsigned write_bytes=write(disk_fd_list_[partition_id],block_for_storing->getBlock(),block_for_storing->getsize());
			if(write_bytes!=block_for_storing->getsize()){
				printf("Error occurs when writing the block into the disk, reason:%s\n",strerror(errno));
			}
			Logging_ExpandableBlockStreamExchangeLM("->Materialize: a block is written to the file!\n");
		}
	}
	/*send the remaining block in the buffer*/
	while(buffer_->getBlockInBuffer()>0){
		const unsigned partition_id=buffer_->getBlock(*block_for_storing);
		const unsigned write_bytes=write(disk_fd_list_[partition_id],block_for_storing->getBlock(),block_for_storing->getsize());
		if(write_bytes!=block_for_storing->getsize()){
			printf("Error occurs when writing the block into the disk, reason:%s\n",strerror(errno));
		}
	}
	/* the child iterator is exhausted and the all the block in the buffer have been store in the file.*/
	Logging_ExpandableBlockStreamExchangeLM("closing the local file!");
	Logging_ExpandableBlockStreamExchangeLM("the local files are closed!");
	return true;
}

void* ExpandableBlockStreamExchangeLowerMaterialized::materialize_and_send(void* arg){
	ExpandableBlockStreamExchangeLowerMaterialized* Pthis=(ExpandableBlockStreamExchangeLowerMaterialized*)arg;
	if(!Pthis->Materialize()){
		perror("Error occurs when materializing the intermediate result!\n");
	}
	Logging_ExpandableBlockStreamExchangeLM("The intermediate is completely stored in the disk!\n");
	Pthis->Send();
	return 0;
}

void* ExpandableBlockStreamExchangeLowerMaterialized::debug(void* arg){
	ExpandableBlockStreamExchangeLowerMaterialized* Pthis=(ExpandableBlockStreamExchangeLowerMaterialized*)arg;
	while(true){
		printf("%d blocks in buffer.\n",Pthis->buffer_->getBlockInBuffer());
		usleep(100000);
	}
	return 0;
}
