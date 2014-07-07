/*
 * CMysqlRowPacket.h
 *
 *  Created on: 2014-6-26
 *      Author: casa
 */

#ifndef C_MYSQL_ROW_PACKET_H_
#define C_MYSQL_ROW_PACKET_H_

#include "c_mysql_sql_packet.h"
#include <stdint.h>

class CMysqlRowPacket: public CMysqlSQLPacket {
public:
	CMysqlRowPacket();
	virtual ~CMysqlRowPacket();

    int serialize(char* buffer, int64_t length, int64_t& pos);
    virtual uint64_t get_serialize_size() {
    	return 0;
    };

private:
    const string row_;
};

#endif /* CMYSQLROWPACKET_H_ */
