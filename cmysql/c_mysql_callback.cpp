/*
 * CMysqlCallback.cpp
 *
 *  Created on: 2014-5-22
 *      Author: casa
 */

#include "c_mysql_callback.h"
#include "c_mysql_command_packet.h"
#include "c_mysql_spr_packet.h"

int CMysqlCallback::debug_ = 0;

CMysqlCallback::CMysqlCallback() {

}

CMysqlCallback::~CMysqlCallback() {

}

int CMysqlCallback::encode(easy_request_t* r, void* packet) {
	cout<<"在CMysqlCallback的encode函数中！"<<endl;
    int ret = 0;
    if (NULL == r || NULL == packet)
    {
      cout<<"error!"<<endl;
      ret = -1;
    }
    else
    {
      easy_buf_t* buf = reinterpret_cast<easy_buf_t*>(packet);
      easy_request_addbuf(r, buf);
      cout<<"加入了缓冲区队列！"<<endl;
    }
    return ret;
}

void* CMysqlCallback::decode(easy_message_t* m) {
	cout<<"在CMysqlCallback的decode函数中！"<<endl;
//	int ret;
//	CMysqlSPRPacket packet;
//	char *buffer=(char *)malloc(2*1024*1024);
//	ObDataBuffer out_buffer(buffer,2*1024*1024);
//	ret=packet.serialize(out_buffer.get_data(),out_buffer.get_capacity(),out_buffer.get_position());
////	ret=write_data(m->fd,out_buffer.get_data(),out_buffer.get_position());
//	return &packet;

    uint32_t pkt_len = 0;
    uint8_t pkt_seq = 0;
    uint8_t pkt_type = 0;
    CMysqlCommandPacket* packet = 0;
    char* buffer = 0;
    int32_t len = 0;

    len = static_cast<int32_t>(m->input->last - m->input->pos);

    CMysqlUtil::get_uint3(m->input->pos, pkt_len);
    CMysqlUtil::get_uint1(m->input->pos, pkt_seq);
    cout<<"SQL: "<<m->input->pos<<endl;

//    message has enough buffer
    if (pkt_len <= m->input->last - m->input->pos)
    {
      CMysqlUtil::get_uint1(m->input->pos, pkt_type);
//      利用message带的pool进行应用层内存的分配
      buffer = reinterpret_cast<char*>(easy_pool_alloc(m->pool,static_cast<uint32_t>(sizeof(CMysqlCommandPacket) + pkt_len)));
      packet = new(buffer)CMysqlCommandPacket();
      packet->set_header(pkt_len, pkt_seq);
      packet->set_type(pkt_type);
      packet->set_receive_ts(0);
      memcpy(buffer + sizeof(CMysqlCommandPacket), m->input->pos, pkt_len - 1);
      m->input->pos += pkt_len-1;

//      packet->get_command().assign(buffer + sizeof(ObMySQLCommandPacket), pkt_len - 1);
//        if (PACKET_RECORDER_FLAG)
//        {
//          // record the packet to FIFO stream if required
//          ObMySQLServer* server = reinterpret_cast<ObMySQLServer*>(m->c->handler->user_data);
//          ObMySQLCommandPacketRecord record;
//          record.socket_fd_ = m->c->fd;
//          record.cseq_ = m->c->seq;
//          record.addr_ = m->c->addr;
//          record.pkt_length_ = pkt_len;
//          record.pkt_seq_ = pkt_seq;
//          record.cmd_type_ = pkt_type;
//          struct iovec buffers[2];
//          buffers[0].iov_base = &record;
//          buffers[0].iov_len = sizeof(record);
//          buffers[1].iov_base = m->input->pos;
//          buffers[1].iov_len = pkt_len - 1;
//          int err = OB_SUCCESS;
//          if (OB_SUCCESS != (err = server->get_packet_recorder().push(buffers, 2)))
//          {
//            TBSYS_LOG(WARN, "failed to record MySQL packet, err=%d", err);
//          }
//        }
//        m->input->pos += pkt_len - 1;
    }
//    else
//    {
//      m->next_read_len = static_cast<int>(pkt_len - (m->input->last - m->input->pos));
//      TBSYS_LOG(DEBUG, "not enough data in message, packet length = %u, data in message is %ld",
//                pkt_len, m->input->last - m->input->pos);
//      m->input->pos -= OB_MYSQL_PACKET_HEADER_SIZE;
//    }
    return packet;
}

int CMysqlCallback::on_connect(easy_connection_t* c) {
	cout<<"在CMysqlCallback的on_connect函数中！"<<endl;
	int ret=0;
	if(NULL==c||NULL==c->handler){
		cout<<"链接处理出错在on_connect函数"<<endl;
	}
	else{
		CMysqlServer* server = reinterpret_cast<CMysqlServer*>(c->handler->user_data);
		ret=server->login_handler(c);
		if(0!=ret){
			ret=1;
			cout<<"登录错误！"<<endl;
		}
		else{
			ret=0;
		}
	}
	return ret;
}

int CMysqlCallback::on_disconnect(easy_connection_t* c) {
	cout<<"在CMysqlCallback的on_disconnect函数中！"<<endl;
	return 0;
}

int CMysqlCallback::process(easy_request_t* r) {
	cout<<"在CMysqlCallback的process函数中！"<<endl;

	int ret;
	easy_pool_set_lock(r->ms->pool);
	cout<<"----进入handle_packet_queue处理，然后返回CMysqlSPRPacket报文！----待补充"<<endl;
	CMysqlServer* server = reinterpret_cast<CMysqlServer*>(r->ms->c->handler->user_data);
	Args *ar=new Args();
	ar->cserver=server;
	ar->r=r;
	server->threadpool->Thread_Pool_add(&(server->do_com_query),ar);
	r->ms->c->pool->ref++;
	easy_atomic_inc(&r->ms->pool->ref);
	ret=-11;
	cout<<"already put the task into threads pool!"<<endl;
	return ret;
}

uint64_t CMysqlCallback::get_packet_id(easy_connection_t* c, void* packet) {
	cout<<"在CMysqlCallback的get_packet_id函数中！"<<endl;
	return 0;
}

int CMysqlCallback::clean_up(easy_request_t *r, void *apacket) {
	cout<<"在CMysqlCallback的clean_up函数中！"<<endl;
	return 0;
}
