/*
 * CMysqlFieldPacket.cpp
 *
 *  Created on: 2014-6-26
 *      Author: casa
 */

#include "c_mysql_field_packet.h"
#include "c_mysql_util.h"

CMysqlFieldPacket::CMysqlFieldPacket():catalog_("def"),
db_(""),length_(0x20000),flags_(0),decimals_(0),charsetnr_(28){
	string tname_="claims";
	string org_tname_="claims";
	string cname_="claims";
	string org_cname_="claims";
	string type_="claims";
}

CMysqlFieldPacket::~CMysqlFieldPacket() {

}

int CMysqlFieldPacket::serialize(char *buf, int64_t len, int64_t& pos) {
	cout<<"serialize a mysql field packet!"<<endl;
	CMysqlUtil::EMySQLFieldType type; //init with type unsupported
	int ret = C_SUCCESS;
	uint8_t num_decimals = decimals_;
	uint32_t length = len;
	ret = CMysqlUtil::get_mysql_type(type, num_decimals, length);
	ret = CMysqlUtil::store_str(buf, len, catalog_, pos);
	ret = CMysqlUtil::store_str(buf, len, db_, pos);
	ret = CMysqlUtil::store_str_v(buf, len, tname_.c_str(), tname_.length(), pos);
	ret = CMysqlUtil::store_str_v(buf, len, org_tname_.c_str(), org_tname_.length(), pos);
	ret = CMysqlUtil::store_str_v(buf, len, cname_.c_str(), cname_.length(), pos);
	ret = CMysqlUtil::store_str_v(buf, len, org_cname_.c_str(), org_cname_.length(), pos);
	ret = CMysqlUtil::store_int1(buf, len, 0xc, pos);
	ret = CMysqlUtil::store_int2(buf, len, charsetnr_, pos);
	ret = CMysqlUtil::store_int4(buf, len, length, pos);
	ret = CMysqlUtil::store_int1(buf, len, type, pos);
	ret = CMysqlUtil::store_int2(buf, len, flags_, pos);
	ret = CMysqlUtil::store_int1(buf, len, num_decimals, pos);
	ret = CMysqlUtil::store_int2(buf, len, 0, pos);
	return 0;
}
