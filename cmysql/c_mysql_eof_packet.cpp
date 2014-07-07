/*
 * CMysqlEOFPacket.cpp
 *
 *  Created on: 2014-6-26
 *      Author: casa
 */

#include "c_mysql_eof_packet.h"

CMysqlEOFPacket::CMysqlEOFPacket()
:field_count_(0xfe),
 warning_count_(0),
 server_status_(0){

}

CMysqlEOFPacket::~CMysqlEOFPacket() {

}

int CMysqlEOFPacket::serialize(char *buffer, int64_t len, int64_t& pos) {
	int ret=C_SUCCESS;
	ret = CMysqlUtil::store_int1(buffer, len, field_count_, pos);
	ret = CMysqlUtil::store_int2(buffer, len, warning_count_, pos);
	ret = CMysqlUtil::store_int2(buffer, len, server_status_, pos);
    return ret;
}

