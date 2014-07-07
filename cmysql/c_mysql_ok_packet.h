/*
 * CMysqlOKPacket.h
 *
 *  Created on: 2014-5-28
 *      Author: casa
 */

#ifndef C_MYSQL_OK_PACKET_H_
#define C_MYSQL_OK_PACKET_H_

#include <stdint.h>

#include "c_mysql_util.h"

class CMysqlOKPacket {
public:
	CMysqlOKPacket();
	virtual ~CMysqlOKPacket();

public:
	int encode(char* buffer, int64_t length, int64_t& pos);
	int serialize(char* buffer, int64_t length, int64_t& pos);

private:
    uint8_t  field_count_;
    uint64_t affected_rows_;
    uint64_t insert_id_;
    uint16_t server_status_;
    uint16_t warning_count_;
};

#endif /* CMYSQLOKPACKET_H_ */
