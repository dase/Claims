/*
 * CMysqlCommandPacket.h
 *
 *  Created on: 2014-6-12
 *      Author: casa
 */

#ifndef C_MYSQL_COMMAND_PACKET_H_
#define C_MYSQL_COMMAND_PACKET_H_

#include <string.h>
#include <stdint.h>

#include "c_define.h"
#include "c_mysql_util.h"

class CMysqlCommandPacket {
public:
	CMysqlCommandPacket();
	virtual ~CMysqlCommandPacket();

    inline void set_header(uint32_t pkt_length, uint8_t pkt_seq)
    {
      length_ = pkt_length;
      seq_ = pkt_seq;
    }

    inline void set_type(uint8_t type)
    {
      type_ = type;
    }

    void set_receive_ts(const int64_t &now)
    {
      receive_ts_ = 0;
    }
private:
    uint32_t length_;
    uint8_t seq_;
    uint8_t type_;
    uint64_t receive_ts_;
};

#endif /* CMYSQLCOMMANDPACKET_H_ */
