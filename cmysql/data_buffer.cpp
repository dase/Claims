/*
 * DataBuffer.cpp
 *
 *  Created on: 2014-5-23
 *      Author: casa
 */

#include "data_buffer.h"

//
//ThreadSpecificBuffer::ThreadSpecificBuffer(const int32_t size):key_(4294967295U),size_(size){
//	create_thread_key();
//}
//
//ThreadSpecificBuffer::~ThreadSpecificBuffer(){
//	delete_thread_key();
//}
//
//int ThreadSpecificBuffer::create_thread_key(){
//	int ret = pthread_key_create(&key_, destroy_thread_key);
//	if(0!=ret){
//		cout<<"create_thread_key"<<endl;
//	}
//	return (0==ret)?0:1;
//}
//
//int ThreadSpecificBuffer::delete_thread_key(){
//	int ret=-1;
//	if(4294967295U!=key_){
//		ret=pthread_key_delete(key_);
//	}
//	if(0!=ret){
//		cout<<"delete thread key key_ failed."<<endl;
//	}
//	return (0 == ret) ? 0 : 1;
//}
//
//void ThreadSpecificBuffer::destroy_thread_key(void* ptr){
//	if(NULL!=ptr)
//		ob_free(ptr);
//}
//
//ThreadSpecificBuffer::Buffer* ThreadSpecificBuffer::get_buffer() const{
//	Buffer *buffer=NULL;
//	if(4294967295U!=key_&&size_>0){
//		void* ptr = pthread_getspecific(key_);
//		if(NULL==ptr){
//			ptr=ob_malloc(size_ + sizeof(Buffer), ObModIds::OB_THREAD_BUFFER);
//			if(NULL!=ptr){
//				int ret = pthread_setspecific(key_, ptr);
//				if (0 != ret){
//					TBSYS_LOG(ERROR, "pthread_setspecific failed:%d", ret);
//					ob_free(ptr);
//					ptr = NULL;
//				}
//				else{
//					TBSYS_LOG(DEBUG, "new thread specific buffer, addr=%p size=%d this=%p", ptr, size_, this);
//					buffer=new (ptr)Buffer(static_cast<char*>(ptr) + sizeof(Buffer), size_);
//				}
//			}
//			else{
//				cout<<"malloc thread specific memeory failed."<<endl;
//			}
//		}
//		else{
//			buffer = reinterpret_cast<Buffer*>(ptr);
//		}
//	}
//	else{
//		TBSYS_LOG(ERROR, "thread key must be initialized and size must great than zero, key:%u,size:%d", key_, size_);
//	}
//	return buffer;
//}
//
//int ThreadSpecificBuffer::Buffer::advance(const int32_t size){
//	int ret=0;
//	if(size<0){
//		if(end_+size<start_){
//			ret = 1;
//		}
//	}
//	else{
//		if(end_+size>end_of_storage_){
//			ret=1;
//		}
//	}
//	if(0==ret)
//		end_+=size;
//	assert(end_>=start_&&end_<=end_of_storage_);
//	return ret;
//}
//
//void ThreadSpecificBuffer::Buffer::reset(){
//	end_ = start_;
//}
//
//int ThreadSpecificBuffer::Buffer::write(const char* bytes, const int32_t size){
//	int ret=0;
//	if(NULL==bytes||size<0){
//		ret=1;
//	}
//	else{
//		if(size>remain())
//			ret=1;
//		else{
//			::memcpy(end_, bytes, size);
//			advance(size);
//		}
//	}
//	return ret;
//}
