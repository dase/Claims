
#ifndef HTTP_CLAMISHTTPSERVER_HPP
#define HTTP_CLAMISHTTPSERVER_HPP


#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.hpp"
#include "../Config.h"

namespace httpserver {

int hargc;
std::string hargv[5];

void httpserver_init(){
	hargc = 5;
	hargv[0] = "";
	sleep(1);
	hargv[1] = Config::httpserver_master_ip;
	hargv[2] = Config::httpserver_master_port;
	hargv[3] = Config::httpserver_thread_num;
	hargv[4] = Config::httpserver_doc_root;
}
int init(int iargc, std::string iargv[]){
	try{
		std::size_t num_threaders = boost::lexical_cast<std::size_t>(iargv[3]);
		httpserver::server s(iargv[1], iargv[2], iargv[4], num_threaders);
		std::cout<<"The httplistener is running!\r\nThe master ip is: "<<hargv[1]<<"\r\nThe master port is:"<<hargv[2]<<"\r\nThe doc_root is "<<hargv[4]<<std::endl;
		s.run();
	//std::cout<<"the main support multithreaded.\n";
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}
	return 0;
}





}



#endif
