/*
 * CMysqlHandshakePacket.cpp
 *
 *  Created on: 2014-5-22
 *      Author: casa
 */

#include "c_mysql_handshake_packet.h"

CMysqlHandshakePacket::CMysqlHandshakePacket() {
    const char* str = "5.";
    protocol_version_ = 10;//Protocol::HandshakeV10
    string server_version(str);
    server_version_=server_version;
    thread_id_=100;//
    memset(scramble_buff_,'a',9);
    server_capabilities_=63487;
    server_language_=8;//latin1_swedish_ci
    server_status_=0;// no this value in mysql protocol document
    memset(plugin_,0,sizeof(plugin_));
    memset(plugin2_,'b',12);
    terminated_='\0';
}

CMysqlHandshakePacket::~CMysqlHandshakePacket() {

}

void CMysqlHandshakePacket::set_thread_id(uint32_t id) {
	thread_id_=id;
}

int CMysqlHandshakePacket::serialize(char *buffer,int64_t len,int64_t &pos) {
	/*------------------报文体-----------------           字节数      实现方式
	 *protocol_version_: 协议版本号                         1       store_int1
	 *server_version_: 服务器版本信息                        3
	 *thread_id_: 服务器线程号                              4        store_int4
	 *scramble_buff_: "ob中将挑战随机数和填充值合并了"         9        memcpy
	 *server_capabilities_: 服务器权能标志                   2       store_int2
	 *server_language_: 字符编码                           1        store_int1
	 *server_status_: 服务器状态                            2       store_int2
    *char plugin_[13]: "ob中服务器权能，挑战长度和填充值的和"  13        memcpy
    *char plugin2_[12]: 第二波挑战随机数，至少12字节         12        memcpy
	 *terminated_: 结束                                   1        memset(0)
	 *------------------报文头-----------------
	 *下面是要放到报文头中
	 *(header size)pkt_len: 消息长度				          3        store_int3
	 *(header seq)0： 消息序号                              1        store_int1
	 [casa@casa ~]$ mysql -h 10.11.1.174 -P2345 --debug-info
		ERROR 2013 (HY000): Lost connection to MySQL server at 'reading authorization packet', system error: 0
		User time 0.00, System time 0.00
		Maximum resident set size 2012, Integral resident set size 0
		Non-physical pagefaults 611, Physical pagefaults 0, Swaps 0
		Blocks in 0 out 0, Messages in 0 out 0, Signals 0
		Voluntary context switches 2, Involuntary context switches 2
	 *(header
	 */
	cout<<"开始序列化报文体和报文头！"<<endl;
	int64_t pkt_len=48;
	CMysqlUtil::store_int3(buffer,len,static_cast<uint32_t>(pkt_len),pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int1(buffer,len,0,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int1(buffer,len,protocol_version_,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	/*服务器版本信息*/
//	cout<<"服务器版本信息长度： "<<server_version_.length()<<endl;
//	memcpy(buffer+pos,server_version_.c_str(),3);pos=pos+3;
	CMysqlUtil::store_str_vzt(buffer,len,server_version_.c_str(),server_version_.length(),pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int4(buffer,len,thread_id_,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	memcpy(buffer+pos,scramble_buff_,9);pos=pos+9;
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int2(buffer,len,server_capabilities_,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int1(buffer,len,server_language_,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	CMysqlUtil::store_int2(buffer,len,server_status_,pos);
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	memcpy(buffer+pos,plugin2_,13);pos=pos+13;
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	memcpy(buffer+pos,plugin2_,12);pos=pos+12;
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	memset(buffer+pos,terminated_,0);pos=pos+1;
//	cout<<"-----------------------------------pos: "<<pos<<endl;
	return 0;
	//三个字节的 length=48;
	//一个字节的 seq=0;
}
