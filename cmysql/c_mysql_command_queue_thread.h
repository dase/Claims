/*
 * CMysqlCommandQueueThread.h
 *
 *  Created on: 2014-5-19
 *      Author: casa
 */

#ifndef C_MYSQL_COMMAND_QUEUE_THREAD_H_
#define C_MYSQL_COMMAND_QUEUE_THREAD_H_

#include <iostream>
using namespace std;

class CMysqlCommandQueueThread {
public:
	CMysqlCommandQueueThread();
	virtual ~CMysqlCommandQueueThread();

public:
	void init();
};

#endif /* CMYSQLCOMMANDQUEUETHREAD_H_ */
