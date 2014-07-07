/*
 * CMysqlSPRPacket.cpp
 *
 *  Created on: 2014-6-12
 *      Author: casa
 */

#include "c_mysql_spr_packet.h"

CMysqlSPRPacket::CMysqlSPRPacket()
: status_(0x05), statement_id_(0),
  column_num_(0), param_num_(0),
  reserved_(0), warning_count_(0){

}

CMysqlSPRPacket::~CMysqlSPRPacket() {

}


int CMysqlSPRPacket::serialize(char *buffer, int64_t len, int64_t &pos) {
	int ret;
//	ret = CMysqlUtil::store_int3(buffer, len, 12, pos);
//	ret = CMysqlUtil::store_int1(buffer, len, 4, pos);
//	pos=pos+4;
	ret = CMysqlUtil::store_int1(buffer, len, status_, pos);
	ret = CMysqlUtil::store_int4(buffer, len, statement_id_, pos);
	ret = CMysqlUtil::store_int2(buffer, len, column_num_, pos);
	ret = CMysqlUtil::store_int2(buffer, len, param_num_, pos);
	ret = CMysqlUtil::store_int1(buffer, len, reserved_, pos);
	ret = CMysqlUtil::store_int2(buffer, len, warning_count_, pos);
	return ret;
}
