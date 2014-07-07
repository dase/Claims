/*
 * CMysqlHandshakePacket.h
 *
 *  Created on: 2014-5-22
 *      Author: casa
 */

#ifndef C_MYSQL_HANDSHAKE_PACKET_H_
#define C_MYSQL_HANDSHAKE_PACKET_H_

#include <string>
#include <iostream>
using namespace std;

#include <string.h>
#include <stdint.h>

#include "c_define.h"
#include "c_mysql_util.h"

class CMysqlHandshakePacket {
public:
	CMysqlHandshakePacket();
	virtual ~CMysqlHandshakePacket();

public:
	void set_thread_id(uint32_t id);
	int serialize(char *buffer,int64_t len,int64_t &pos);

private:
    uint8_t protocol_version_;
    string server_version_;
    uint32_t thread_id_;//connection id
    char scramble_buff_[8];// auth_plugin_data_part_1 : first 8 bytes of the auth-plugin data
    uint16_t server_capabilities_;  /* set value to use 4.1protocol */
    uint8_t server_language_;
    uint16_t server_status_;
    char plugin_[13];        /* always 0x00 */
    char plugin2_[12];
    char terminated_;

    string str_buf_;   //store ObString content



};

#endif /* CMYSQLHANDSHAKEPACKET_H_ */
