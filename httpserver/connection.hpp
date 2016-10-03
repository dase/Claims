//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "../Daemon/Daemon.h"
#include <boost/thread/mutex.hpp>

namespace httpserver {

class ResultString{
public:
	vector<int> fd_;
	vector<int> sock_fd_;
	vector<bool> connection_lock_;
	vector<bool> result_got_;
	vector<ExecutedResult> result_;
	const int connection_max_number_ = 100;
};

static boost::mutex mutex_;



inline ResultString& GetResultString(){
	static ResultString st;
	return st;
}



class connection_manager;

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
      //connection_manager& manager,
	  request_handler& handler);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  /// Start the first asynchronous operation for the connection.
  void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();


private:
  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e);

  /// Handle the url and count the ";".
  bool scmd_handle(const std::string str);

  /// Socket for the connection.
  boost::asio::ip::tcp::socket socket_;
  /// The manager for this connection.
  //connection_manager& connection_manager_;

  /// The handler used to process the incoming request.
  request_handler& request_handler_;

  /// Buffer for incoming data.
  boost::array<char, 8192> buffer_;

  /// The incoming request.
  request request_;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  reply reply_;

};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace http

#endif // HTTP_CONNECTION_HPP
