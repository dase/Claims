/*
 * CMysqlSQLPacket.cpp
 *
 *  Created on: 2014-7-1
 *      Author: casa
 */

#include "c_mysql_sql_packet.h"

CMysqlSQLPacket::CMysqlSQLPacket() {

}

CMysqlSQLPacket::~CMysqlSQLPacket() {

}

int CMysqlSQLPacket::encode(char* buffer, int64_t length, int64_t& pos){
	cout<<"encoding a packet!"<<endl;
	int ret = C_SUCCESS;
	int64_t start_pos = pos;
	pos += 4;
	ret = serialize(buffer, length, pos);
	int number=5;
	if (C_SUCCESS == ret)
	{
	  uint32_t pkt_len = static_cast<uint32_t>(pos - start_pos - 4);
	  ret = CMysqlUtil::store_int3(buffer, length, pkt_len, start_pos);
	  ret = CMysqlUtil::store_int1(buffer, length, number, start_pos);
	}
	else
	{
		cout<<"encode occurred a error!"<<endl;
	}
	return ret;
}
