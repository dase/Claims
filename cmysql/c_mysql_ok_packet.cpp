/*
 * CMysqlOKPacket.cpp
 *
 *  Created on: 2014-5-28
 *      Author: casa
 */

#include "c_mysql_ok_packet.h"

#include <iostream>
using namespace std;

CMysqlOKPacket::CMysqlOKPacket()
:field_count_(0x00),
 affected_rows_(0),
 insert_id_(0),
 server_status_(0x02),
 warning_count_(0){

}

CMysqlOKPacket::~CMysqlOKPacket() {

}

int CMysqlOKPacket::encode(char* buffer, int64_t length, int64_t& pos){
	int ret = C_SUCCESS;

	int64_t start_pos = pos;
	pos += 4;
	ret = serialize(buffer,length,pos);
	uint32_t pkt_len = static_cast<uint32_t>(pos-start_pos-4);
	ret = CMysqlUtil::store_int3(buffer,length,pkt_len,start_pos);
	ret = CMysqlUtil::store_int1(buffer,length,2,start_pos);
	return ret;
}

int CMysqlOKPacket::serialize(char* buffer, int64_t length, int64_t& pos){
	int ret = C_SUCCESS;
	/*
	 * ==================报文体============
	 * OK报文，值恒为0x00
	 * 受影响的行数0
	 * 索引ID值0
	 * 服务器状态2
	 * 警告计数0
	 * 服务器消息（可选）
	 * */
//	cout<<field_count_<<endl;
	ret = CMysqlUtil::store_int1(buffer,length,field_count_,pos);
//	cout<<affected_rows_<<endl;
	ret = CMysqlUtil::store_length(buffer,length,affected_rows_,pos);
//	cout<<insert_id_<<endl;
	ret = CMysqlUtil::store_length(buffer,length,insert_id_,pos);
//	cout<<server_status_<<endl;
	ret = CMysqlUtil::store_int2(buffer,length,server_status_,pos);
//	cout<<warning_count_<<endl;
	ret = CMysqlUtil::store_int2(buffer,length,warning_count_,pos);
//	cout<<"ret: "<<buffer[0]<<"-"<<buffer[1]<<"-"<<buffer[2]<<"-"<<buffer[3]<<"-"<<buffer[4]<<"-"<<buffer[5]<<"-"<<buffer[6]<<endl;
	string str(buffer);
//	cout<<"传输前的报文内容： "<<str.length()<<endl;
	const char *hello="hello";
	memcpy(buffer+pos,hello,5);
	pos=pos+5;
//	ret = CMysqlUtil::store_obstr(buffer,length,message_,pos);
	return ret;
}
