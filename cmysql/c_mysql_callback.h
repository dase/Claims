/*
 * CMysqlCallback.h
 *
 *  Created on: 2014-5-22
 *      Author: casa
 */

#ifndef C_MYSQL_CALLBACK_H_
#define C_MYSQL_CALLBACK_H_

#include <iostream>
using namespace std;

#include "c_mysql_server.h"

#include "easy_io_struct.h"

class CMysqlCallback {
public:
	CMysqlCallback();
	virtual ~CMysqlCallback();

public:
	static int encode(easy_request_t* r, void* packet);
	static void* decode(easy_message_t* m);
	static int on_connect(easy_connection_t* c);
	static int on_disconnect(easy_connection_t* c);
	static int process(easy_request_t* r);
	static uint64_t get_packet_id(easy_connection_t* c, void* packet);
	static int clean_up(easy_request_t *r, void *apacket);

private:
	static int debug_;

};

#endif /* CMYSQLCALLBACK_H_ */
