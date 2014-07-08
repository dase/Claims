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
    enum EMySQLFieldType
    {
      MYSQL_TYPE_DECIMAL,
      MYSQL_TYPE_TINY,
      MYSQL_TYPE_SHORT,
      MYSQL_TYPE_LONG,
      MYSQL_TYPE_FLOAT,
      MYSQL_TYPE_DOUBLE,
      MYSQL_TYPE_NULL,
      MYSQL_TYPE_TIMESTAMP,
      MYSQL_TYPE_LONGLONG,
      MYSQL_TYPE_INT24,
      MYSQL_TYPE_DATE,
      MYSQL_TYPE_TIME,
      MYSQL_TYPE_DATETIME,
      MYSQL_TYPE_YEAR,
      MYSQL_TYPE_NEWDATE,
      MYSQL_TYPE_VARCHAR,
      MYSQL_TYPE_BIT,
      MYSQL_TYPE_NEWDECIMAL=246,
      MYSQL_TYPE_ENUM=247,
      MYSQL_TYPE_SET=248,
      MYSQL_TYPE_TINY_BLOB=249,
      MYSQL_TYPE_MEDIUM_BLOB=250,
      MYSQL_TYPE_LONG_BLOB=251,
      MYSQL_TYPE_BLOB=252,
      MYSQL_TYPE_VAR_STRING=253,
      MYSQL_TYPE_STRING=254,
      MYSQL_TYPE_GEOMETRY=255,
      MYSQL_TYPE_NOT_DEFINED=65535
    };

	CMysqlUtil();
	virtual ~CMysqlUtil();

public:
	static int store_null(char *buf, int64_t len, int64_t &pos);
	static int store_int1(char *buf,int64_t len,int8_t v,int64_t &pos);
	static int store_int2(char *buf,int64_t len,int16_t v,int64_t &pos);
	static int store_int3(char *buf,int64_t len,int32_t v,int64_t &pos);
	static int store_int4(char *buf,int64_t len,int32_t v,int64_t &pos);
	static int store_int8(char *buf, int64_t len, int64_t v, int64_t &pos);
	static int store_str(char *buf, int64_t len, const char *str, int64_t &pos);
	static int store_length(char *buf, int64_t len, uint64_t length, int64_t &pos);

	static int store_str_v(char *buf, int64_t len, const char *str,const uint64_t length, int64_t &pos);
	static int store_str_vzt(char *buf, int64_t len, const char *str,const uint64_t length, int64_t &pos);

    static void get_uint3(char *&pos, uint32_t &v);
    static void get_uint4(char *&pos, uint32_t &v);
    static void get_uint1(char *&pos, uint8_t &v);

    static int get_mysql_type(EMySQLFieldType &,uint8_t &,uint32_t &);

};

#endif /* CMYSQLUTIL_H_ */
