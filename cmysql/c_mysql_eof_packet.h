/*
 * CMysqlEOFPacket.h
 *
 *  Created on: 2014-6-26
 *      Author: casa
 */

#ifndef C_MYSQL_EOF_PACKET_H_
#define C_MYSQL_EOF_PACKET_H_

#include <stdint.h>
#include "c_mysql_util.h"
#include "c_mysql_sql_packet.h"

class CMysqlEOFPacket: public CMysqlSQLPacket {
public:
	CMysqlEOFPacket();
	virtual ~CMysqlEOFPacket();

	int serialize(char *buffer,int64_t len, int64_t &pos);

    inline void set_warning_count(uint16_t count)
    {
      warning_count_ = count;
    }

    inline void set_server_status(uint16_t status)
    {
      server_status_ = status;
    }

    virtual uint64_t get_serialize_size()
    {
    	uint64_t len = 0;
        len += 1;                 // field_count_
        len += 2;                 // warning_count_
        len += 2;                 // server_status_
        return len;
    }

    //for test
    inline uint8_t get_field_count() const
    {
      return field_count_;
    }

    inline uint16_t get_warning_count() const
    {
      return warning_count_;
    }

    inline uint16_t get_server_status() const
    {
      return server_status_;
    }

private:
    uint8_t field_count_;
    uint16_t warning_count_;
    uint16_t server_status_;
};

#endif /* CMYSQLEOFPACKET_H_ */
