/*
    Copyright 2011, Spyros Blanas.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hashtable.h"

#include <malloc.h>
#include "rdtsc.h"


BasicHashTable::BasicHashTable(unsigned nbuckets, unsigned bucksize, unsigned tuplesize)
:nbuckets_(nbuckets), tuplesize_(tuplesize)
{

	try
	{
		overflow_count_=new unsigned[nbuckets];
		memset(overflow_count_,0,nbuckets*sizeof(unsigned));

		/** create spinlocks, each of which corresponds to a single bucket*/
		lock_list_=new SpineLock[nbuckets];
//		for(unsigned i=0;i<nbuckets_;i++){
//			lock_list_[i]=new SpineLock();
//		}

		bucket_ = (void**)new char[sizeof(void*) * nbuckets];
		memset(bucket_,0,sizeof(void*) * nbuckets);
		bucksize_=get_aligned_space(bucksize);
		if(bucksize_<tuplesize+2*sizeof(void*)){
			bucksize_=tuplesize+2*sizeof(void*);
		}
		buck_actual_size_=bucksize_-2*sizeof(void*);
		pagesize_=(unsigned long)nbuckets*bucksize_<16*1024*(unsigned long)1024?(unsigned long)nbuckets*bucksize_:16*1024*(unsigned long)1024;
//		printf("Page size:%d\n",pagesize_);
		char* cur_mother_page=(char*)memalign(PAGE_SIZE,pagesize_);
		assert(cur_mother_page);
		t_start_=cur_mother_page;

		/*
		 * It's beneficial in IBM X5 to reset the newly allocated memory.
		 * TODO: check whether other servers benefit from this.
		 * Finished TODO. --Dec 31th by li
		 * memset should not be used in our new machines.
		 */
		memset(cur_mother_page,0,pagesize_);
		assert(cur_mother_page);
		cur_MP_=0;
		mother_page_list_.push_back(cur_mother_page);

		for (unsigned i=0; i<nbuckets; i++)
		{
			if(bucksize_+cur_MP_>=pagesize_)// the current mother page doesn't have enough space for the new buckets
			{
				cur_mother_page=(char*)memalign(PAGE_SIZE,pagesize_);
				assert(cur_mother_page);

				//TODO: as mentioned above.
				memset(cur_mother_page,0,pagesize_);

				cur_MP_=0;
				mother_page_list_.push_back(cur_mother_page);
			}
			bucket_[i]=(void*)(cur_mother_page+cur_MP_);
			cur_MP_+=bucksize_;

			void** free = (void**)(((char*)bucket_[i]) + buck_actual_size_);
			void** next = (void**)(((char*)bucket_[i]) + buck_actual_size_ + sizeof(void*));
			*next = 0;
			*free = bucket_[i];
		}
	}
	catch (exception& e) {
		cout << "failed" << endl;
		cout << "EXCEPTION: Standard exception: " << e.what() << endl;}
}

BasicHashTable::~BasicHashTable()
{
	for(int i=0;i<mother_page_list_.size();i++)
	{
		free(mother_page_list_.at(i));
	}
		delete[] bucket_;
		delete[] lock_list_;
}

BasicHashTable::Iterator::Iterator() : buck_actual_size(0), tuplesize(0), cur(0), next(0), free(0) { }
BasicHashTable::Iterator::Iterator(const unsigned& buck_actual_size,const unsigned& tuplesize):buck_actual_size(buck_actual_size),tuplesize(tuplesize),cur(0), next(0), free(0){}

void* BasicHashTable::allocate(const unsigned & offset){
//	assert(offset<nbuckets_);

	void* data=bucket_[offset];
	void* ret;
	if(data>0){
		void** freeloc = (void**)((char*)data + buck_actual_size_);

		if ((*freeloc)+tuplesize_ <= ((char*)data + buck_actual_size_))
		{
			ret = *freeloc;
			*freeloc = ((char*)(*freeloc)) + tuplesize_;
//			assert(ret!=0);
			return ret;
		}
	}

	mother_page_lock_.lock();
	char* cur_mother_page=mother_page_list_.back();
	if(bucksize_+cur_MP_>=pagesize_ )// the current mother page doesn't have enough space for the new buckets
	{
		cur_mother_page=(char*)memalign(PAGE_SIZE,pagesize_);
		//TODO: as mentioned in .cpp.
		memset(cur_mother_page,0,pagesize_);
		assert(cur_mother_page);
		cur_MP_=0;
		mother_page_list_.push_back(cur_mother_page);
	}
	overflow_count_[offset]++;
	ret=cur_mother_page+cur_MP_;
	cur_MP_+=bucksize_;
	mother_page_lock_.unlock();

	void** new_buck_nextloc = (void**)(((char*)ret) + buck_actual_size_ + sizeof(void*));
	void** new_buck_freeloc = (void**)(((char*)ret) + buck_actual_size_);
	*new_buck_freeloc = (ret)+tuplesize_ ;
	*new_buck_nextloc = data;

	bucket_[offset]=ret;
	return ret;
}

void BasicHashTable::report_status() {
	printf("-----------Hash table status--------------\n");
	printf("Bucket size: %d\n",bucksize_);
	printf("#. of buckets: %d\n",nbuckets_);
	printf("Number of Mother pages:%d\n",mother_page_list_.size());
	unsigned long int total_overflow_buckets=0;
	for(unsigned i=0;i<nbuckets_;i++){
		total_overflow_buckets+=overflow_count_[i];
	}
	printf("#. of overflow buckets: %ld\n",total_overflow_buckets);
	printf("------------------------------------------\n");

}

BasicHashTable::Iterator BasicHashTable::CreateIterator() const
{
	return BasicHashTable::Iterator(buck_actual_size_,tuplesize_);
}
BasicHashTable::Iterator::Iterator(const Iterator& r){
	this->buck_actual_size=r.buck_actual_size;
	this->cur=r.cur;
	this->free=r.free;
	this->next=r.next;
	this->tuplesize=r.tuplesize;
}
BasicHashTable::Iterator::~Iterator(){
	this->buck_actual_size=0;
	this->cur=0;
	this->free=0;
	this->next=0;
	this->tuplesize=0;

}

