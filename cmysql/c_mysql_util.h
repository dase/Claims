/*
 * CMysqlUtil.h
 *
 *  Created on: 2014-5-27
 *      Author: casa
 */

#ifndef C_MYSQL_UTIL_H_
#define C_MYSQL_UTIL_H_

#include "c_define.h"

#include <stdint.h>
#include <string.h>

class CMysqlUtil {
public:
	CMysqlUtil();
	virtual ~CMysqlUtil();

public:
	static int store_null(char *buf, int64_t len, int64_t &pos);
	static int store_int1(char *buf,int64_t len,int8_t v,int64_t &pos);
	static int store_int2(char *buf,int64_t len,int16_t v,int64_t &pos);
	static int store_int3(char *buf,int64_t len,int32_t v,int64_t &pos);
	static int store_int4(char *buf,int64_t len,int32_t v,int64_t &pos);
	static int store_int8(char *buf, int64_t len, int64_t v, int64_t &pos);
	static int store_length(char *buf, int64_t len, uint64_t length, int64_t &pos);

	static int store_str_vzt(char *buf, int64_t len, const char *str,const uint64_t length, int64_t &pos);


    static void get_uint3(char *&pos, uint32_t &v);
    static void get_uint4(char *&pos, uint32_t &v);
    static void get_uint1(char *&pos, uint8_t &v);

};

#endif /* CMYSQLUTIL_H_ */
