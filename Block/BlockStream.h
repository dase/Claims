/*
 * BlockStream.h
 *
 *  Created on: Aug 26, 2013
 *      Author: wangli
 */

#ifndef BLOCKSTREAM_H_
#define BLOCKSTREAM_H_
#include "../Schema/Schema.h"
#include "Block.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "../rename.h"
using namespace std;

class BlockStreamBase:public Block{
public:

	class BlockStreamTraverseIterator{
	public:
		explicit BlockStreamTraverseIterator(BlockStreamBase* block_stream_base)
		:block_stream_base_(block_stream_base),cur(0){};
		~BlockStreamTraverseIterator(){};
		inline void* nextTuple(){
			return block_stream_base_->getTuple(cur++);
		}


		/* get the current tuple of the iterator without increasing cur_
		 * Usually, increase_cur_() is called after.
		 */
		inline void* currentTuple() const{
			return block_stream_base_->getTuple(cur);
		}


		/* This function is usually called after currentTuple()  */
		inline void increase_cur_(){
			cur++;
		}


		inline void reset(){
			cur=0;
		}

	private:
		BlockStreamBase* block_stream_base_;
		unsigned cur;
	};
	friend class BlockStreamTraverseIterator;
	virtual ~BlockStreamBase(){};

	virtual void* allocateTuple(unsigned bytes)=0;
	virtual void setEmpty()=0;
	virtual bool Empty() const =0;
	virtual void* getBlockDataAddress()=0;
//	virtual void setBlockDataAddress(void* addr)=0;
	virtual unsigned getTuplesInBlock()const=0;
	/* copy a block in the storage layer into the BlockStream, the member variables (e.g., _free) are
	 * also updated according the new data.
	 */
	virtual void copyBlock(void* addr, unsigned length)=0;

	virtual void constructFromBlock(const Block& block)=0;

	virtual bool switchBlock(BlockStreamBase &block)=0;

	/* serialize the Block Stream into the Block which can be sent through the network.*/
	virtual bool serialize(Block &block) const =0;

	/* convert the Block from the network into the content of current instance*/
	virtual bool deserialize(Block *block)=0;

	/* return the actual size to hold this stream block, including the data and some auxiliary structure.*/
	virtual unsigned getSerializedBlockSize()const=0;

	BlockStreamTraverseIterator* createIterator(){
		return new BlockStreamTraverseIterator(this);
	};
	BlockStreamBase(unsigned block_size):Block(block_size){};
	static BlockStreamBase* createBlock(Schema* schema,unsigned block_size);
	/*for debug*/
	virtual void printSchema()=0;
	virtual bool insert(void *dest,void *src,unsigned bytes)=0;
	virtual void toDisk(BlockStreamBase *bsb)=0;
protected:
	virtual void* getTuple(unsigned offset) const =0;

};

class BlockStreamFix:public BlockStreamBase {
public:
	BlockStreamFix(unsigned block_size,unsigned tuple_size);
	virtual ~BlockStreamFix();
public:
	inline void* allocateTuple(unsigned bytes){
		if(free_+bytes<=start+BlockSize){
			void* ret=free_;
			free_+=bytes;
			return ret;
		}
		return 0;
	}
	void setEmpty();

	/* get [offset]-th tuple of the block */
	inline void* getTuple(unsigned offset) const {
		void* ret=start+offset*tuple_size_;
		if(ret>=free_){
			return 0;
		}
		return ret;
	}
	bool Empty() const;
	void* getBlockDataAddress();
//	void setBlockDataAddress(void* addr);
	bool switchBlock(BlockStreamBase& block);
	void copyBlock(void* addr, unsigned length);
	bool serialize(Block & block) const;
	bool deserialize(Block * block);
	unsigned getSerializedBlockSize()const;
	unsigned getTuplesInBlock()const;

	/* construct the BlockStream from a storage level block,
	 * which last four bytes indicate the number of tuples in the block.*/
	void constructFromBlock(const Block& block);
	virtual void printSchema(){};
	bool insert(void *dest,void *src,unsigned bytes){}
	void toDisk(BlockStreamBase *bsb){};
protected:
//	char* data_;
//	unsigned block_size_;
	char* free_;
	unsigned tuple_size_;
public:

};

/* only on the situation of tuple size is little than a block size*/
class BlockStreamVar:public BlockStreamBase{
public:
	BlockStreamVar(unsigned block_size,Schema *schema);
	virtual ~BlockStreamVar();

	inline void* getTuple(unsigned offset) const {
		// 首先通过offset得到第offset个tuple的起始地址
		void *ret=start+BlockSize-4-(offset+1)*4;
		return ret;
	}
public:
//	void *getCurrentEnd(){
//		// a member of class can be used to reduce computations
//		return (char*)start+BlockSize-sizeof(int)*(cur_tuple_size_+1+attributes_);
//	}
//
//	void *getCurrentFront(){
//		unsigned begin=*(int*)((char*)start+BlockSize-sizeof(int)*(cur_tuple_size_+attributes_));
//		unsigned varsize=0;
//		for(unsigned i=0;i<var_attributes_;i++){
//			varsize+=*(int *)(begin+i);
//		}
//		unsigned begin=(int*)((char*)start+begin);
//		return (char *)(begin+varsize+4*var_attributes_);
//	}

	/* bytes is the length of the constructed tuple*/
	inline void* allocateTuple(unsigned bytes){
		if(free_front_+bytes<=free_end_){
			void *ret=free_front_;
			free_front_+=bytes;
			return ret;
		}
		else{
			cout<<"the allocateTuple is not success!!!"<<endl;
			return 0;
		}
	}

	bool insert(void *dest,void *src,unsigned bytes){
//		cout<<"error in insert"<<endl;
		memcpy(dest,src,bytes);
		int *free_end=(int*)free_end_;
		*free_end=free_front_-start;
		free_end_=free_end_-sizeof(int);
//		cout<<"cur_tuple_size_:"<<cur_tuple_size_++<<endl;
		return true;
	}

	void setEmpty(){};

	bool Empty() const{};
	void* getBlockDataAddress(){};
//	void setBlockDataAddress(void* addr);
	bool switchBlock(BlockStreamBase& block){};
	void copyBlock(void* addr, unsigned length){
	};
	bool serialize(Block & block) const{
		*(int*)((char*)start+BlockSize-4)=cur_tuple_size_;
	};
	bool deserialize(Block * block){};
	unsigned getSerializedBlockSize()const{};
	unsigned getTuplesInBlock()const{};
	void constructFromBlock(const Block& block){};
	void toDisk(BlockStreamBase *bsb){
		int filed;
		filed=FileOpen("/home/casa/storage/file/file_01",O_WRONLY|O_CREAT);
		write(filed,bsb->getBlock(),1024*64);
	};

	/*debug*/
public:
	void printSchema(){
		int columns=schema_->getncolumns();
		int* schema_info=(int*)((char*)start+BlockSize-sizeof(int)*(columns+1));
		for(unsigned i=0;i<schema_->getncolumns();i++){
			cout<<"the schema is:"<<*(schema_info+i)<<endl;
		}
		cout<<"the tuple count is:"<<*(schema_info+columns)<<endl;
	}
private:
	// front can be added for less computing
//	char *front_;
	Schema *schema_;
	char *free_front_;
	/* can be in end directly*/
	char *free_end_;
	/* how many attributes in the tuple*/
	unsigned attributes_;
	unsigned var_attributes_;
	unsigned cur_tuple_size_;
};


#endif /* BLOCKSTREAM_H_ */
