/*
 * CMysqlResHeaderPacket.cpp
 *
 *  Created on: 2014-6-26
 *      Author: casa
 */

#include "c_mysql_res_header_packet.h"

CMysqlResHeaderPacket::CMysqlResHeaderPacket(): field_count_(0) {

}

CMysqlResHeaderPacket::~CMysqlResHeaderPacket() {

}

int CMysqlResHeaderPacket::serialize(char* buffer, int64_t len, int64_t& pos) {
    int ret = C_SUCCESS;
    ret = CMysqlUtil::store_length(buffer, len, field_count_, pos);
    return ret;
}

inline uint64_t CMysqlResHeaderPacket::get_field_count() const {
	return field_count_;
};

uint64_t CMysqlResHeaderPacket::get_serialize_size(){
	uint64_t len = 0;
	/* MAX_STORE_LENGTH */
	len += 9; // field_count_
	return len;
}
