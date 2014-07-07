/*
 * CMysqlUtil.cpp
 *
 *  Created on: 2014-5-27
 *      Author: casa
 */

#include "c_mysql_util.h"

#include <string.h>

#include <iostream>
using namespace std;

#define UINT64_MAX 123413334

CMysqlUtil::CMysqlUtil() {

}

CMysqlUtil::~CMysqlUtil() {

}

int CMysqlUtil::store_null(char *buf, int64_t len, int64_t &pos){
  return store_int1(buf, len, static_cast<int8_t>(251), pos);
}


int CMysqlUtil::store_int1(char *buf,int64_t len,int8_t v,int64_t &pos){
	int ret=C_SUCCESS;
	if(len>=pos+1){
		int1store(buf+pos,v);
        pos+=1;
	}
    else{
        ret=C_SIZE_OVERFLOW;
    }
    return ret;
}

int CMysqlUtil::store_int2(char *buf, int64_t len, int16_t v, int64_t &pos){
  int ret = C_SUCCESS;

  if (len >= pos + 2)
  {
    int2store(buf + pos, v);
    pos += 2;
  }
  else
  {
    ret = C_SIZE_OVERFLOW;
  }
  return ret;
}

int CMysqlUtil::store_int3(char *buf,int64_t len,int32_t v,int64_t &pos){
	int ret = C_SUCCESS;

	if (len >= pos + 3){
		int3store(buf + pos, v);
		pos += 3;
	}

	else{
		ret = C_SIZE_OVERFLOW;
	}

	return ret;
}

int CMysqlUtil::store_int4(char *buf, int64_t len, int32_t v, int64_t &pos){
  int ret = C_SUCCESS;

  if (len >= pos + 4)
  {
    int4store(buf + pos, v);
    pos += 4;
  }
  else
  {
    ret = C_SIZE_OVERFLOW;
  }

  return ret;
}

int CMysqlUtil::store_int8(char *buf, int64_t len, int64_t v, int64_t &pos){
  int ret = C_SUCCESS;

  if (len >= pos + 8)
  {
    int8store(buf + pos, v);
    pos += 8;
  }
  else
  {
    ret = C_SIZE_OVERFLOW;
  }

  return ret;
}

int CMysqlUtil::store_length(char *buf, int64_t len, uint64_t length, int64_t &pos){
   int ret = C_SUCCESS;
   if (len < 0 || pos < 0 || len <= pos)
   {
     ret = C_SIZE_OVERFLOW;
   }

   int64_t remain = len - pos;
   if (C_SUCCESS == ret)
   {
     if (length < (uint64_t) 251 && remain >= 1)
     {
       ret = store_int1(buf, len, (uint8_t) length, pos);
     }
     /* 251 is reserved for NULL */
     else if (length < (uint64_t) 0X10000 && remain >= 3)
     {
       ret = store_int1(buf, len, static_cast<int8_t>(252), pos);
       if (C_SUCCESS == ret)
       {
         ret = store_int2(buf, len, (uint16_t) length, pos);
         if (C_SUCCESS != ret)
         {
           pos--;
         }
       }
     }
     else if (length < (uint64_t) 0X1000000 && remain >= 4)
     {
       ret = store_int1(buf, len, (uint8_t) 253, pos);
       if (C_SUCCESS == ret)
       {
         ret = store_int3(buf, len, (uint32_t) length, pos);
         if (C_SUCCESS != ret)
         {
           pos--;
         }
       }
     }

     else if (length < UINT64_MAX && remain >= 9)
     {
       ret = store_int1(buf, len, (uint8_t) 254, pos);
       if (C_SUCCESS == ret)
       {
         ret = store_int8(buf, len, (uint64_t) length, pos);
         if (C_SUCCESS != ret)
         {
           pos--;
         }
       }
     }
     else if (length == UINT64_MAX) /* NULL_ == UINT64_MAX */
     {
       ret = store_null(buf, len, pos);
     }
     else
     {
       ret = C_SIZE_OVERFLOW;
     }
   }

   return ret;
 }

int CMysqlUtil::store_str_vzt(char *buf, int64_t len, const char *str,const uint64_t length, int64_t &pos){
  int ret = C_SUCCESS;
  if (len > 0 && pos > 0 && len > pos && static_cast<uint64_t>(len - pos) > length){
    memcpy(buf + pos, str, length);
    pos += length;
    buf[pos++] = '\0';
  }
  else
  {
    ret = C_SIZE_OVERFLOW;
    //TBSYS_LOG(WARN, "Store string fail, buffer over flow!"
    //          " len: [%ld], pos: [%ld], length: [%ld], ret: [%d]",
    //          len, pos, length, ret);
  }
  return ret;
}

void CMysqlUtil::get_uint3(char *&pos, uint32_t &v){
  v = uint3korr(pos);
  pos += 3;
}

void CMysqlUtil::get_uint4(char *&pos, uint32_t &v){
  v = uint4korr(pos);
  pos += 4;
}

void CMysqlUtil::get_uint1(char *&pos, uint8_t &v){
  v = uint1korr(pos);
  pos ++;
}
