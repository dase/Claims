/*
 * CMysqlSQLPacket.h
 *
 *  Created on: 2014-7-1
 *      Author: casa
 */

#ifndef C_MYSQL_SQL_PACKET_H_
#define C_MYSQL_SQL_PACKET_H_

#include "c_define.h"
#include "c_mysql_util.h"

#include <iostream>
using namespace std;

#include <stdint.h>

class CMysqlSQLPacket {
public:
	CMysqlSQLPacket();
	virtual ~CMysqlSQLPacket();

	int encode(char* buffer, int64_t length, int64_t& pos);
	virtual int serialize(char* buffer, int64_t length, int64_t& pos)=0;
};

#endif /* CMYSQLSQLPACKET_H_ */
