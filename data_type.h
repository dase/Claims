/*
 * data_type.h
 *
 *  Created on: May 12, 2013
 *      Author: wangli
 */

#ifndef DATA_TYPE_H_
#define DATA_TYPE_H_
#include <assert.h>
#include <string.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "hash.h"
enum data_type{t_int,t_float,t_string,t_double,t_u_long};
typedef void (*fun)(void*,void*);


template<typename T>
inline void ADD(void* target, void* increment)
{
	*(T*)target=*(T*)increment;
}
template<>
inline void ADD<char*>(void* target, void* increment)
{
}

template<typename T>
inline void MIN(void* target, void* increment)
{
	if(*(T*)target>*(T*)increment)
		*(T*)target=*(T*)increment;
}
template<>
inline void MIN<char*>(void* target, void* increment)
{

}
template<typename T>
inline void MAX(void* target, void* increment)
{
	if(*(T*)target<*(T*)increment)
		*(T*)target=*(T*)increment;
}
template<>
inline void MAX<char*>(void* target, void* increment)
{

}

template<typename T>
inline void IncreaseByOne(void* target,void* increment)
{
	(*(T*)target)++;
}
template<>
inline void IncreaseByOne<char*>(void* target,void* increment)
{

}

template<typename T>
inline void assigns(const void* src, void* desc){
	*(T*)desc=*(T*)src;
}
template<>
inline void assigns<char*>(const void* src,void* desc){
	strcpy((char*)desc,(char*)src);
}
class Operate
{
public:
	Operate(){};
//	virtual ~Operate(){};
	inline void ass(void* src, void* desc){
		*(int*)desc=*(int*)src;
	}
	inline virtual void assignment(const void* src,void* &desc) const =0;
	virtual unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const=0;
	virtual unsigned getPartitionValue(const void* key)const=0;
	virtual std::string toString(void* value)=0;
	virtual bool equal(void* a, void* b)=0;
	virtual void add(void* target, void* increment)=0;
	virtual fun GetADDFunction()=0;
	virtual fun GetMINFunction()=0;
	virtual fun GetMAXFunction()=0;
	virtual fun GetIncreateByOneFunction()=0;
	void (*assign)(const void*,void*);
};

class OperateInt:public Operate
{
public:
	OperateInt(){assign=assigns<int>;};
//	~OperateInt(){};
	inline void assignment(const void* src,void* &desc)const
	{
		*(int*)desc=*(int*)src;
	};
	inline std::string toString( void* value)
	{
		std::ostringstream ss;
		ss<<*(int*)value;
		std::string ret=ss.str();
		return ret;
	};
	inline bool equal(void* a, void* b)
	{
		return *(int*)a==*(int*)b;
	}
	inline void add(void* target, void* increment)
	{
		ADD<int>(target,increment);
	}
	inline fun GetADDFunction()
	{
		return ADD<int>;
	}
	inline fun GetMINFunction()
	{
		return MIN<int>;
	}
	inline fun GetMAXFunction()
	{
		return MAX<int>;
	}
	inline fun GetIncreateByOneFunction()
	{
		return IncreaseByOne<int>;
	}
	unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const{
		return partition_function->get_partition_value(*(int*)key);
	}
	unsigned getPartitionValue(const void* key)const{
		return boost::hash_value(*(int*)key);
	}
};

class OperateFloat:public Operate
{
public:
	OperateFloat(){};
//	~OperateFloat(){};
	inline void assignment(const void* src,void* &desc)const
	{
		*(float*)desc=*(float*)src;
	};
	inline std::string toString(void* value)
	{
		std::ostringstream ss;
		ss<<*(float*)value;
		std::string ret=ss.str();
		return ret;
	};
	inline bool equal(void* a, void* b)
	{
		return *(float*)a==*(float*)b;
	}
	inline void add(void* target, void* increment)
	{
		ADD<float>(target, increment);
	}
	inline fun GetADDFunction()
	{
		return ADD<float>;
	}
	inline fun GetMINFunction()
	{
		return MIN<float>;
	}
	inline fun GetMAXFunction()
	{
		return MAX<float>;
	}
	inline fun GetIncreateByOneFunction()
	{
		return IncreaseByOne<float>;
	}
	unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const{
		return partition_function->get_partition_value(*(float*)key);
	}
	unsigned getPartitionValue(const void* key)const{
		return boost::hash_value(*(float*)key);
	}
};

class OperateDouble:public Operate
{
public:
	OperateDouble(){};
//	~OperateDouble(){};
	inline void assignment(const void* src,void* &desc)const
	{
		*(double*)desc=*(double*)src;
	};
	inline std::string toString(void* value)
	{
		std::ostringstream ss;
		ss<<*(double*)value;
		std::string ret=ss.str();
		return ret;
	};
	inline bool equal(void* a, void* b)
	{
		return *(double*)a==*(double*)b;
	}
	inline void add(void* target, void* increment)
	{
		ADD<double>(target, increment);
	}
	inline fun GetADDFunction()
	{
		return ADD<double>;
	}
	inline fun GetMINFunction()
	{
		return MIN<double>;
	}
	inline fun GetMAXFunction()
	{
		return MAX<double>;
	}
	inline fun GetIncreateByOneFunction()
	{
		return IncreaseByOne<double>;
	}
	unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const{
		return partition_function->get_partition_value(*(double*)key);
	}
	unsigned getPartitionValue(const void* key)const{
		return boost::hash_value(*(double*)key);
	}
};

class OperateULong:public Operate
{
public:
	OperateULong(){};
//	~OperateULong(){};
	inline void assignment(const void* src,void* &desc)const
	{
		*(unsigned long*)desc=*(unsigned long*)src;
	};
	inline std::string toString(void* value)
	{
		std::ostringstream ss;
		ss<<*(unsigned long*)value;
		std::string ret=ss.str();
		return ret;
	};
	inline bool equal(void* a, void* b)
	{
		return *(unsigned long*)a==*(unsigned long*)b;
	}
	inline void add(void* target, void* increment)
	{
		ADD<unsigned long>(target, increment);
	}
	inline fun GetADDFunction()
	{
		return ADD<unsigned long>;
	}
	inline fun GetMINFunction()
	{
		return MIN<unsigned long>;
	}
	inline fun GetMAXFunction()
	{
		return MAX<unsigned long>;
	}
	inline fun GetIncreateByOneFunction()
	{
		return IncreaseByOne<unsigned long>;
	}
	unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const{
		return partition_function->get_partition_value(*(unsigned long*)key);
	}
	unsigned getPartitionValue(const void* key)const{
		return boost::hash_value(*(unsigned long*)key);
	}
};

class OperateString:public Operate
{
public:
	OperateString(){};
//	~OperateString(){};
	inline void assignment(const void *src,void* &desc)const
	{
		assert(desc!=0&&src!=0);
		strcpy((char*)desc,(char*)src);
	};
	inline std::string toString(void* value)
	{
		return std::string((char*)value);
	};
	inline bool equal(void* a, void* b)
	{
		return strcmp((char*)a,(char*)b)==0;
	}
	inline void add(void* target, void* increment)
	{
		//TODO: throw exception or implement the add for string.
		printf("The sum for String is not current supported!\n");
	}
	inline fun GetADDFunction()
	{
		return ADD<char*>;
	}
	inline fun GetMINFunction()
	{
		return MIN<char*>;
	}
	inline fun GetMAXFunction()
	{
		return MAX<char*>;
	}
	inline fun GetIncreateByOneFunction()
	{
		return IncreaseByOne<char*>;
	}
	unsigned getPartitionValue(const void* key,const PartitionFunction* partition_function)const{
		printf("The hash function for char[] type is not implemented yet!\n");
		assert(false);

		return 0;
	}
	unsigned getPartitionValue(const void* key)const{
		return boost::hash_value(*(char*)key);
	}
};

class column_type
{
public:
	 column_type(data_type type,unsigned _size=0):type(type),size(size){
		switch(type)
		{
			case t_int:operate=new OperateInt();break;
			case t_float:operate=new OperateFloat();break;
			case t_string:operate=new OperateString();break;
			case t_double:operate=new OperateDouble();break;
			case t_u_long:operate=new OperateULong();break;
			default:operate=0;break;
		}
	};
	column_type():operate(0){};
//	~column_type(){};
	inline unsigned get_length() const
	{
		switch(type)
		{
			case t_int: return sizeof(int);
			case t_float:return sizeof(float);
			case t_double:return sizeof(double);
			case t_u_long:return sizeof(unsigned long);
			case t_string:return size;
			default: return 0;

		}
	}
	bool operator==(const class column_type &c) const
	{
		return this->type==c.type;
	}
	bool operator<(const class column_type &c) const
	{

		return this->type<c.type;
	}
public:
	Operate* operate;
	data_type type;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & type & size;
		if(operate==0)
		{
			initialize();
		}
	}
	/**
	 * called after deserialization to construct the right operator.
	 */
	void initialize()
	{
		switch(type)
		{
			case t_int:operate=new OperateInt();break;
			case t_float:operate=new OperateFloat();break;
			case t_double:operate=new OperateDouble();break;
			case t_string:operate=new OperateString();break;
			case t_u_long:operate=new OperateULong();break;
			default:operate=0;break;
		}
	}
private:

	unsigned size;

};


#endif /* DATA_TYPE_H_ */
