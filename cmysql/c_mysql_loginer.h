/*
 * CMysqlLoginer.h
 *
 *  Created on: 2014-5-22
 *      Author: casa
 */

#ifndef C_MYSQL_LOGINER_H_
#define C_MYSQL_LOGINER_H_

#include "easy_io.h"

#include "c_mysql_ok_packet.h"
#include "data_buffer.h"

class CMysqlServer;

class CMysqlLoginer {
public:
	CMysqlLoginer();
	virtual ~CMysqlLoginer();

public:
	int login(easy_connection_t* c);
	int handshake(easy_connection_t* c);
	int parse_packet(easy_connection_t* c);
	int check_privilege(easy_connection_t* c);

	void set_c_server(CMysqlServer *server);
	int write_data(int fd,char *buffer,size_t length);
	int read_data(int fd, char* buffer,size_t length);

private:
	CMysqlServer *server_;
//	ThreadSpecificBuffer buffer_;
};

#endif /* CMYSQLLOGINER_H_ */
