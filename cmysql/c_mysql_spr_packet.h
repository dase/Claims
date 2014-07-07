/*
 * CMysqlSPRPacket.h
 *
 *  Created on: 2014-6-12
 *      Author: casa
 */

#ifndef C_MYSQL_SPR_PACKET_H_
#define C_MYSQL_SPR_PACKET_H_

#include <string>
#include <iostream>
using namespace std;

#include <string.h>
#include <stdint.h>

#include "c_define.h"
#include "c_mysql_util.h"

class CMysqlSPRPacket {
public:
	CMysqlSPRPacket();
	virtual ~CMysqlSPRPacket();

public:
	int serialize(char *buffer, int64_t length, int64_t &pos);

    inline void set_statement_id(uint32_t id)
    {
      statement_id_ = id;
    }

    inline void set_column_num(uint16_t num)
    {
      column_num_ = num;
    }

    inline void set_param_num(uint16_t num)
    {
      param_num_ = num;
    }

    inline void set_warning_count(uint16_t count)
    {
      warning_count_ = count;
    }

    uint64_t get_serialize_size()
    {
      uint64_t len = 0;
      len += 1;                 // status
      len += 4;                 // statement id
      len += 2;                 // column num
      len += 2;                 // param num
      len += 1;                 // reserved
      len += 2;                 // warning count
      return len;
    }

private:
    uint8_t  status_;
    uint32_t statement_id_;
    uint16_t column_num_;
    uint16_t param_num_;
    uint8_t  reserved_;
    uint16_t warning_count_;
};

#endif /* CMYSQLSPRPACKET_H_ */
