/*
 * CMysqlServer.cpp
 *
 *  Created on: 2014-5-19
 *      Author: casa
 */

#include "c_mysql_server.h"
#include "c_mysql_field_packet.h"
#include "c_mysql_eof_packet.h"

CMysqlServer::CMysqlServer() {
	threadpool=new Thread_Pool();
	threadpool->Thread_Pool_init(2,2);
}

CMysqlServer::~CMysqlServer() {

}

int CMysqlServer::start() {
	cout<<"启动cliams mysql server..."<<endl;
	int ret;
	int rc;
	easy_listen_t* listen = NULL;

	eio_=easy_eio_create(eio_,io_threads_count_);
    eio_->do_signal = 0;
    //OB_CONNECTION_FREE_TIME_S的时间
    eio_->force_destroy_second = 900;
    eio_->checkdrc = 1;
    eio_->support_ipv6 = 0;
    eio_->no_redispatch = 1;
    easy_eio_set_uthread_start(eio_, easy_on_ioth_start, this);
    eio_->uthread_enable = 0;
	command_queue_thread_.init();
	eio_->tcp_defer_accept = 0;
	ret=initialize();
    if(NULL==(listen=easy_connection_add_listen(eio_, NULL, port_, &handler_))){
    	cout<<"启动监听错误！"<<endl;
    }else{
    	cout<<"监听已经启动，端口号为"<<port_<<endl;
    }

    rc = easy_eio_start(eio_);

    //这里的原先为EASY_OK == rc,先假设EASY_OK是0
    if(0==rc){
    	cout<<"启动成功！"<<endl;
    }
    else{
    	cout<<"启动失败！"<<endl;
    }

    //是否需要等待？
    if(0==ret){
        easy_eio_wait(eio_);
    }
    return ret;
}

int CMysqlServer::initialize() {
	cout<<"初始化服务器！"<<endl;
	//为什么OB在此是没有初始化日志服务器的呢？
	login_hander_=new CMysqlLoginer();
    memset(&handler_, 0, sizeof(easy_io_handler_pt));
    //ObMySQLCallback这个类要我么来实现，回调类
    handler_.encode=CMysqlCallback::encode;//ObMySQLCallback::encode;
    handler_.decode=CMysqlCallback::decode;//ObMySQLCallback::decode;
    handler_.process=CMysqlCallback::process;//ObMySQLCallback::process;
    handler_.get_packet_id=CMysqlCallback::get_packet_id;//ObMySQLCallback::get_packet_id;
    handler_.on_disconnect=CMysqlCallback::on_disconnect;//ObMySQLCallback::on_disconnect;
    //on_connect函数有一个监听的方法，这个监听的方法在监听登录的用户
    handler_.on_connect=CMysqlCallback::on_connect;//ObMySQLCallback::on_connect;
    handler_.cleanup=CMysqlCallback::clean_up;//ObMySQLCallback::clean_up;
    handler_.user_data=this;
    //这里要封装成函数
    io_threads_count_=20;
    port_=2345;

    return 0;
}

void CMysqlServer::on_ioth_start() {
  //配置文件传值
  int64_t affinity_start_cpu=1;
  int64_t affinity_end_cpu=2;
  //检查传值是否正确
  if (0<=affinity_start_cpu&& affinity_start_cpu<= affinity_end_cpu){
    static volatile int cpu = 0;
    //下面这个函数是否在标准库中？
    int64_t local_cpu=__sync_fetch_and_add(&cpu, 1) % (affinity_end_cpu - affinity_start_cpu + 1) + affinity_start_cpu;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(local_cpu, &cpuset);
    int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  }
}

int CMysqlServer::login_handler(easy_connection_t * c) {
	cout<<"登录处理"<<endl;
	login_hander_->set_c_server(this);
	int ret;
	ret=login_hander_->login(c);
	return ret;
}

void CMysqlServer::do_com_query(void *arg){
	cout<<"OK, I have finished some query and will send the result set back, here type is 1!"<<endl;
	cout<<"=======before is ok!=======in do_com_query"<<endl;
	Args *pthis=reinterpret_cast<Args *>(arg);
	pthis->cserver->send_response(pthis->r,1);
	cout<<"one query is has done!"<<endl;
}

void CMysqlServer::send_response(easy_request_t* req, int type){
	cout<<"send response!"<<endl;
	cout<<"在此模拟result set然后发送！"<<endl;
    easy_addr_t addr = get_easy_addr(req);
//    int number = packet->get_packet_header().seq_;
    //在此先构造一下server_status
    uint16_t server_status =0;
    uint16_t charset=1;
	cout<<"=======before is ok!=======in send_response"<<endl;
	send_result_set(req,type,server_status,charset);
}

void CMysqlServer::send_result_set(easy_request_t *req,int type,uint16_t server_status, uint16_t charset){
	cout<<"send result set!"<<endl;
    int64_t buffer_length = 0;
    int64_t buffer_pos = 0;

	easy_addr_t addr = get_easy_addr(req);
	easy_buf_t* buf = reinterpret_cast<easy_buf_t*>(easy_pool_alloc(req->ms->pool, 2*1024*1024));

    char *data_buffer = reinterpret_cast<char *>(buf + 1);
    buffer_length = 2*1024*1024 - sizeof(easy_buf_t);
    init_easy_buf(buf, data_buffer, req, buffer_length);

    /* result 在这里模拟result */
	cout<<"=======before is ok!=======in send_result_set"<<endl;
	cout<<">>>>>>>>>>>>>>send header packet"<<endl;
	process_res_header_packet(buf, buffer_pos, req);
	cout<<">>>>>>>>>>>>>>send field packet"<<endl;
	process_field_packet(buf, buffer_pos, req, true, 1);
	cout<<">>>>>>>>>>>>>>send eof packet"<<endl;
	process_eof_packet(buf, buffer_pos, req, server_status);
//	process_row_packet(buf, buffer_pos, req, result, type);
	process_eof_packet(buf, buffer_pos, req, server_status);
}

void CMysqlServer::process_eof_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req, uint16_t server_status){
	cout<<"send eof packet!"<<endl;
	int ret=C_SUCCESS;
	CMysqlEOFPacket *eof=new CMysqlEOFPacket();
	ret=process_single_packet(buff,buff_pos,req,eof);
}

void CMysqlServer::process_res_header_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req){
	cout<<"send res header packet!"<<endl;
	int ret=C_SUCCESS;
	CMysqlResHeaderPacket *header=new CMysqlResHeaderPacket();
	/* 在此模拟发送两个列 */
	header->set_field_count(2);
	//getchar();
	/* 设置seq */
	ret=process_single_packet(buff,buff_pos,req,header);
	cout<<"check ret here: "<<ret<<endl;
}

void CMysqlServer::process_field_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req, bool is_field, uint16_t charset){
	cout<<"send field packet!"<<endl;
	int ret=C_SUCCESS;
	easy_addr_t addr=get_easy_addr(req);
	CMysqlFieldPacket *field=new CMysqlFieldPacket();
	ret=process_single_packet(buff,buff_pos,req,field);
}

void CMysqlServer::process_row_packet(){
	cout<<"send row packet!"<<endl;
//	int ret=C_SUCCESS;
//	easy_addr_t addr=get_easy_addr(req);
//	CMysqlRowPacket *row=new CMysqlRowPacket();
//	ret=process_single_packet(buff,buff_pos,req,row);
}

int CMysqlServer::process_single_packet(easy_buf_t *&buff, int64_t &buff_pos, easy_request_t *req, CMysqlSQLPacket *packet){
	int ret=C_SUCCESS;
	easy_addr_t addr=get_easy_addr(req);

//	char *buffer=(char *)malloc(2*1024*1024);
//	ObDataBuffer out_buffer(buffer,2*1024*1024);
	/* 对packet加头 */
//	ret=packet->encode(out_buffer.get_data(),out_buffer.get_capacity(),out_buffer.get_position());

	ret = packet->encode(buff->pos,buff->end-buff->pos,buff_pos);
	buff->last=buff->pos+buff_pos;
	req->opacket=reinterpret_cast<void *>(buff);
	cout<<"=======before is ok!=======in process_single_packet"<<endl;
	//getchar();
	ret=send_raw_packet(req);
	return ret;
}

int CMysqlServer::post_packet(easy_request_t* req, CMysqlSPRPacket* packet, uint8_t seq)
{
	int ret;
//	ObDataBuffer buffer;
	int32_t pkt_len = 0;
	int64_t len_pos = 0;
	int64_t pos = 0;
	uint64_t size = packet->get_serialize_size() + 4;

	char *buffer=(char *)malloc(2*1024*1024);
	ObDataBuffer out_buffer(buffer,2*1024*1024);
//	ret=packet.serialize(out_buffer.get_data(),out_buffer.get_capacity(),out_buffer.get_position());

	//同样，还是在message的pool上分配内存
	easy_buf_t* buf = reinterpret_cast<easy_buf_t*>(easy_pool_alloc(req->ms->pool,static_cast<uint32_t>(sizeof(easy_buf_t) + size)));
	char* buff = 0;//out_buffer.get_data();

	pos += 3;
	buff = reinterpret_cast<char*>(buf + 1);
	init_easy_buf(buf, buff, req, size);
	CMysqlUtil::store_int1(buff, size, seq, pos);
	ret = packet->serialize(buff, size, pos);
	len_pos = 0;
		// 写入包的长度
	pkt_len = static_cast<int32_t>(pos-4);
	CMysqlUtil::store_int3(buff, size, pkt_len, len_pos);
	buf->pos = buff;
	buf->last = buff + pos;
	buf->end  = buff + size;
		//设置output packet
	req->opacket = reinterpret_cast<void*>(buf);
		//hex_dump(buf->pos,  static_cast<int32_t>(buf->last - buf->pos), true, TBSYS_LOG_LEVEL_INFO);
	easy_request_wakeup(req);
	cout<<"pass easy_request_wakeup and next?"<<endl;
	return ret;
}

int CMysqlServer::send_raw_packet(easy_request_t *req)
{
  int ret = C_SUCCESS;
  easy_buf_t *buf = static_cast<easy_buf_t*>(req->opacket);
  if (NULL != buf){
  	  cout<<"this is a ob_stat_inc func!"<<endl;
  }

  easy_client_wait_t wait_obj;
  wait_obj.done_count = 0;
  //用于IO线程唤醒工作线程
  easy_client_wait_init(&wait_obj);
  req->client_wait = &wait_obj;
  req->retcode = -11;
  req->waiting = 1;
  //io线程被唤醒，r->opacket被挂过去,send_response->easy_connection_request_done
  easy_request_wakeup(req);
  // IO线程回调 int ObMySQLCallback::process(easy_request_t* r)的时候唤醒工作线程
  /* 卡在这里的原因是，这里现在还是同步的，不是异步的 */
  wait_client_obj(wait_obj);
  if(wait_obj.status==3){
	  ret=-124;
  }
  easy_client_wait_cleanup(&wait_obj);
  req->client_wait = NULL;
  return ret;
}
