//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <string.h>
#include <boost/lexical_cast.hpp>
//#include "connection_manager.hpp"
#include "request_handler.hpp"
#include "../Daemon/Daemon.h"
#include "../httpserver/json/json.h"
#include "ResultManage.hpp"

namespace httpserver {

connection::connection(boost::asio::io_service& io_service,
		//connection_manager& manager,
		request_handler& handler) :
		socket_(io_service), //connection_manager_(manager),
		request_handler_(
				handler) {
}

boost::asio::ip::tcp::socket& connection::socket() {
	return socket_;
}


void connection::start() {
	socket_.async_read_some(boost::asio::buffer(buffer_),
			boost::bind(&connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}
/*
void connection::stop() {
	//socket_.shutdown(boost::asio::socket_base::shutdown_both);
	socket_.close();
}
*/
bool connection::scmd_handle(std::string str) {
	int temp = 0;
	for (int i = 0; i < str.length(); i++)
		if (str[i] == ';')
			temp++;
	if (temp == 1)
		return true;
	else
		return false;
}

void connection::handle_read(const boost::system::error_code& e,
		std::size_t bytes_transferred) {
	if (!e) {
		boost::tribool result;
		boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
				request_, buffer_.data(), buffer_.data() + bytes_transferred);

		if (result) {
			std::string reqstr;
			int reqint;
			for (int i = 0; i < request_.uri.length(); i++) {
				if (request_.uri[i] == '%') {
					reqint = 0;
					for (int j = i + 1; j <= i + 2; j++) {
						if (request_.uri[j] >= '0' && request_.uri[j] <= '9') {
							reqint = reqint * 16 + request_.uri[j] - '0';
						}
						if (request_.uri[j] >= 'A' && request_.uri[j] <= 'F') {
							reqint = reqint * 16 + request_.uri[j] - 'A' + 10;
						}
					}
					i = i + 2;
					reqstr.push_back(reqint);
				} else
					reqstr.push_back(request_.uri[i]);
			}

			std::string reqstrl = "";
			for (int i = 0; i < 8; i++)
				reqstrl.push_back(reqstr[i]);
			if (reqstrl == "/CLAIMS/") {
				std::string scmd = "";

				for (int i = 8; i < reqstr.length(); i++)
					scmd.push_back(reqstr[i]);

				remote_command rcmd;
				ResultString& rs = GetResultString();


				/*
				int i;
				for (i = 1; i < rs.connection_max_number_; i++) {
					mutex.lock();
					if (rs.connection_lock_[i] == false) {
						rs.connection_lock_[i] = true;
						mutex.unlock();
						rcmd.socket_fd = -rs.fd_[i];
						break;
					}
					mutex.unlock();
				}

				//this function is not be completed.if the link_pool is full there is no handle.
				if (i == rs.connection_max_number_) {

					reply_.status = reply::ok;
					reply_.content.append(
							"The number of connections has reached the upper limit,please try again.");
					reply_.headers.resize(2);
					reply_.headers[0].name = "Content-Length";
					reply_.headers[0].value = boost::lexical_cast<std::string>(
							reply_.content.size());
					reply_.headers[1].name = "Content-Type";
					reply_.headers[1].rsvalue = "text/html";

					boost::asio::async_write(socket_, reply_.to_buffers(),
							boost::bind(&connection::handle_write,
									shared_from_this(),
									boost::asio::placeholders::error));
				}

				else {
				*/



				if (!scmd_handle(scmd)) {

					reply_.status = reply::ok;
					reply_.content.append(
							"Please ensure your url have only one ';'.");
					reply_.headers.resize(2);
					reply_.headers[0].name = "Content-Length";
					reply_.headers[0].value = boost::lexical_cast<
							std::string>(reply_.content.size());
					reply_.headers[1].name = "Content-Type";
					reply_.headers[1].value = "text/html";

					boost::asio::async_write(socket_, reply_.to_buffers(),
							boost::bind(&connection::handle_write,
									shared_from_this(),
									boost::asio::placeholders::error));

				} else {


					int i;
					mutex_.lock();
					for (i = 2; i < rs.connection_max_number_; i++) {
						if (rs.connection_lock_[i] == false) {
							rs.connection_lock_[i] = true;
							rcmd.socket_fd = -rs.fd_[i];
							break;
						}
					}
					mutex_.unlock();

					//this function is not be completed.if the link_pool is full there is no handle.
					if (i == rs.connection_max_number_) {

						reply_.status = reply::ok;
						reply_.content.append(
								"The number of connections has reached the upper limit,please try again.");
						reply_.headers.resize(2);
						reply_.headers[0].name = "Content-Length";
						reply_.headers[0].value = boost::lexical_cast<std::string>(
								reply_.content.size());
						reply_.headers[1].name = "Content-Type";
						reply_.headers[1].value = "text/html";

						boost::asio::async_write(socket_, reply_.to_buffers(),
								boost::bind(&connection::handle_write,
										shared_from_this(),
										boost::asio::placeholders::error));
					}

					else {

						rcmd.cmd = scmd;
						Daemon::getInstance()->addRemoteCommand(rcmd);
						while (true) {
							usleep(100);
							if (rs.result_got_[i] == true) {

								string buff_to_send;
								result_manage(buff_to_send,
										rs.result_[i]);
								ExecutedResult resulttemp;
								mutex_.lock();
								rs.result_[i] = resulttemp;
								rs.result_got_[i] = false;
								rs.connection_lock_[i] = false;
								mutex_.unlock();
								reply_.status = reply::ok;
								reply_.content.append(buff_to_send);
								reply_.headers.resize(2);
								reply_.headers[0].name = "Content-Length";
								reply_.headers[0].value = boost::lexical_cast<
										std::string>(reply_.content.size());
								reply_.headers[1].name = "Content-Type";
								reply_.headers[1].value = "text/html";
								//sleep(100);
								boost::asio::async_write(socket_,
										reply_.to_buffers(),
										boost::bind(&connection::handle_write,
												shared_from_this(),
												boost::asio::placeholders::error));
								break;
							}
						} //the result from claims is in the ResultString rs;
					}
				}
			} else {
				request_handler_.handle_request(request_, reply_);

				boost::asio::async_write(socket_, reply_.to_buffers(),
						boost::bind(&connection::handle_write,
								shared_from_this(),
								boost::asio::placeholders::error));
			}
		} else if (!result) {
			reply_ = reply::stock_reply(reply::bad_request);
			boost::asio::async_write(socket_, reply_.to_buffers(),
					boost::bind(&connection::handle_write, shared_from_this(),
							boost::asio::placeholders::error));
		} else {
			socket_.async_read_some(boost::asio::buffer(buffer_),
					boost::bind(&connection::handle_read, shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
		}
	}/* else if (e != boost::asio::error::operation_aborted) {
		connection_manager_.stop(shared_from_this());
	}*/
}

void connection::handle_write(const boost::system::error_code& e) {
	if (!e) {
		// Initiate graceful connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
				ignored_ec);
	}

	/*if (e != boost::asio::error::operation_aborted) {
		connection_manager_.stop(shared_from_this());
	}*/
}

} // namespace http
