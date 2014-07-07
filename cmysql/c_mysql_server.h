/*
 * CMysqlServer.h
 *
 *  Created on: 2014-5-19
 *      Author: casa
 */

#ifndef C_MYSQL_SERVER_H_
#define C_MYSQL_SERVER_H_

#include <easy_io.h>
#include <easy_io_struct.h>

#include "c_mysql_command_queue_thread.h"
#include "c_mysql_callback.h"
#include "c_mysql_loginer.h"
#include "c_mysql_spr_packet.h"
#include "c_mysql_command_packet.h"
#include "c_mysql_res_header_packet.h"
#include "c_mysql_sql_packet.h"

#include "Thread_Pool.h"

#include <iostream>
using namespace std;

#include <pthread.h>

#define OB_LIKELY(x)       __builtin_expect(!!(x),1)

class CMysqlLoginer;


//send_result_set
//process_row_packets
//send_raw_packet发送最后的结果

struct Args{
	CMysqlServer *cserver;
	easy_request_t *r;
};

//ObMySQLRowPacket
class CMysqlServer {
public:
	CMysqlServer();
	virtual ~CMysqlServer();

public:
    void on_ioth_start();

    int start();
	int initialize();
	int login_handler(easy_connection_t * c);
	int post_packet(easy_request_t* req, CMysqlSPRPacket* packet, uint8_t seq);
	int send_raw_packet(easy_request_t *req);

	static void do_com_query(void *);
	void send_response(easy_request_t *req, int type);
	void send_result_set(easy_request_t *req,int type,uint16_t server_status, uint16_t charset);

	void process_res_header_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req);
	void process_field_packet();
	void process_eof_packet();
	void process_row_packet();

	int process_single_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req, CMysqlSQLPacket *packet);

    static void easy_on_ioth_start(void *arg){
      if (arg != NULL){
        CMysqlServer *server = reinterpret_cast<CMysqlServer*>(arg);
        server->on_ioth_start();
      }
    }

    inline easy_addr_t get_easy_addr(easy_request_t *req)
    {
      static easy_addr_t empty = {0, 0, {0}, 0};
      if (OB_LIKELY(NULL != req
                    && NULL != req->ms
                    && NULL != req->ms->c))
      {
        return req->ms->c->addr;
      }
      else
      {
        return empty;
      }
    }

    inline void init_easy_buf(easy_buf_t *buf, char* data, easy_request_t *req, uint64_t size)
    {
      if (NULL != buf && NULL != data)
      {
        buf->pos = data;
        buf->last = buf->pos;
        buf->end = buf->last + size;
        buf->cleanup = NULL;
        if (NULL != req && NULL != req->ms)
        {
          buf->args = req->ms->pool;
        }
        buf->flags = 0;
        easy_list_init(&buf->node);
      }
    }

    inline void wait_client_obj(easy_client_wait_t& client_wait)
    {
      pthread_mutex_lock(&client_wait.mutex);
      cout<<"222"<<endl;
      if (client_wait.done_count == 0)
      {
      	cout<<"333"<<endl;
        //unlock同时wait在cond上，等待网络框架将其唤醒,唤醒的时候同时获得锁
        pthread_cond_wait(&client_wait.cond, &client_wait.mutex);
      }
  	  cout<<"444"<<endl;
      pthread_mutex_unlock(&client_wait.mutex);
    }

public:
    Thread_Pool *threadpool;

private:
	CMysqlCommandQueueThread command_queue_thread_;
	int io_threads_count_;
	int port_;

    easy_io_t* eio_;
    easy_io_handler_pt handler_;

    CMysqlLoginer *login_hander_;
};

#endif /* CMYSQLSERVER_H_ */
