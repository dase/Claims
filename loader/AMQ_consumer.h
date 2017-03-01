/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /Claims/loader/AMQ_consumer.h
 *
 *  Created on: May 1, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef LOADER_AMQ_CONSUMER_H_
#define LOADER_AMQ_CONSUMER_H_
#include <activemq/transport/DefaultTransportListener.h>
#include <cms/Connection.h>
#include <cms/Destination.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageConsumer.h>
#include <cms/MessageListener.h>
#include <cms/Session.h>
#include <string>

#include "../common/rename.h"
using activemq::transport::DefaultTransportListener;
using cms::CMSException;
using cms::Connection;
using cms::Destination;
using cms::ExceptionListener;
using cms::Message;
using cms::MessageConsumer;
using cms::MessageListener;
using cms::Session;

namespace claims {
namespace loader {

class MasterLoader;

class AMQConsumer : public ExceptionListener,
                    public MessageListener,
                    public DefaultTransportListener {
 public:
  AMQConsumer(const std::string& brokerURI, const std::string& destURI,
              bool use_topic = false, bool client_ack = false)
      : connection_(NULL),
        session_(NULL),
        destination_(NULL),
        consumer(NULL),
        use_topic_(use_topic),
        brokerURI_(brokerURI),
        destURI_(destURI),
        client_ack_(client_ack) {}

  virtual ~AMQConsumer() { this->cleanup(); }

  void close() { this->cleanup(); }

  void run(MasterLoader* mloader);

  virtual void onMessage(const Message* message);

  // If something bad happens you see it here as this class is also been
  // registered as an ExceptionListener with the connection.
  virtual void onException(const CMSException& ex AMQCPP_UNUSED) {
    printf("CMS Exception occurred.  Shutting down client.\n");
    exit(1);
  }

  virtual void onException(const decaf::lang::Exception& ex) {
    printf("Transport Exception occurred: %s \n", ex.getMessage().c_str());
  }

  virtual void transportInterrupted() {
    std::cout << "The Connection's Transport has been Interrupted."
              << std::endl;
  }

  virtual void transportResumed() {
    std::cout << "The Connection's Transport has been Restored." << std::endl;
  }

  NO_COPY_AND_ASSIGN(AMQConsumer);

 private:
  void cleanup();

 public:
 private:
  Connection* connection_;
  Session* session_;
  Destination* destination_;
  MessageConsumer* consumer;
  bool use_topic_;
  std::string brokerURI_;
  std::string destURI_;
  bool client_ack_;

  MasterLoader* mloader_;
};

} /* namespace loader */
} /* namespace claims */

#endif  // LOADER_AMQ_CONSUMER_H_
