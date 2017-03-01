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
 * /ActiveMQ_c/Customer.cpp
 *
 *  Created on: 2016年4月7日
 *      Author: Han
 *		   Email:
 *
 * Description:
 *
 */

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Integer.h>
#include <activemq/util/Config.h>
#include <decaf/util/Date.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>

using std::string;
using namespace std;

using namespace activemq;                 // NOLINT
using namespace activemq::core;           // NOLINT
using namespace activemq::transport;      // NOLINT
using namespace decaf::lang;              // NOLINT
using namespace decaf::util;              // NOLINT
using namespace decaf::util::concurrent;  // NOLINT
using namespace cms;                      // NOLINT

////////////////////////////////////////////////////////////////////////////////
class Consumer : public ExceptionListener,
                 public MessageListener,
                 public DefaultTransportListener {
 private:
  Connection* connection_;
  Session* session_;
  Destination* destination_;
  MessageConsumer* consumer;
  bool use_topic_;
  std::string brokerURI_;
  std::string destURI_;
  bool client_ack_;

 private:
  Consumer(const Consumer&);
  Consumer& operator=(const Consumer&);

 public:
  Consumer(const std::string& brokerURI, const std::string& destURI,
           bool use_topic = false, bool client_ack = false)
      : connection_(NULL),
        session_(NULL),
        destination_(NULL),
        consumer(NULL),
        use_topic_(use_topic),
        brokerURI_(brokerURI),
        destURI_(destURI),
        client_ack_(client_ack) {}

  virtual ~Consumer() { this->cleanup(); }

  void close() { this->cleanup(); }

  void runConsumer() {
    try {
      // Create a ConnectionFactory
      ActiveMQConnectionFactory* connectionFactory =
          new ActiveMQConnectionFactory(brokerURI_);

      // Create a Connection
      connection_ = connectionFactory->createConnection();
      delete connectionFactory;

      ActiveMQConnection* amqConnection =
          dynamic_cast<ActiveMQConnection*>(connection_);
      if (amqConnection != NULL) {
        amqConnection->addTransportListener(this);
      }

      connection_->start();

      connection_->setExceptionListener(this);

      // Create a Session
      if (client_ack_) {
        session_ = connection_->createSession(Session::CLIENT_ACKNOWLEDGE);
      } else {
        session_ = connection_->createSession(Session::AUTO_ACKNOWLEDGE);
      }

      // Create the destination (Topic or Queue)
      if (use_topic_) {
        destination_ = session_->createTopic(destURI_);
      } else {
        destination_ = session_->createQueue(destURI_);
      }

      // Create a MessageConsumer from the Session to the Topic or Queue
      consumer = session_->createConsumer(destination_);
      consumer->setMessageListener(this);

    } catch (CMSException& e) {
      e.printStackTrace();
    }
  }

  // Called from the consumer since this class is a registered MessageListener.
  virtual void onMessage(const Message* message) {
    static int count = 0;
    static timeval start1;
    try {
      const TextMessage* textMessage =
          dynamic_cast<const TextMessage*>(message);
      string text = "";

      if (textMessage != NULL) {
        text = textMessage->getText();
      } else {
        text = "NOT A TEXTMESSAGE!";
      }

      if (client_ack_) {
        message->acknowledge();
        //        cout << "consume a message" << endl;
      }
      // enter for load.
      //      printf("Message #%d Received: %s\n", count, text.c_str());
    } catch (CMSException& e) {
      e.printStackTrace();
    }
  }

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

 private:
  void cleanup() {
    //*************************************************
    // Always close destination, consumers and producers before
    // you destroy their sessions and connection.
    //*************************************************

    // Destroy resources.
    try {
      if (destination_ != NULL) delete destination_;
    } catch (CMSException& e) {
    }
    destination_ = NULL;

    try {
      if (consumer != NULL) delete consumer;
    } catch (CMSException& e) {
    }
    consumer = NULL;

    // Close open resources.
    try {
      if (session_ != NULL) session_->close();
      if (connection_ != NULL) connection_->close();
    } catch (CMSException& e) {
    }

    // Now Destroy them
    try {
      if (session_ != NULL) delete session_;
    } catch (CMSException& e) {
    }
    session_ = NULL;

    try {
      if (connection_ != NULL) delete connection_;
    } catch (CMSException& e) {
    }
    connection_ = NULL;
  }
};

////////////////////////////////////////////////////////////////////////////////
void ClientInitialize() {
  std::cout << "=====================================================\n";
  std::cout << "Starting the example:" << std::endl;
  std::cout << "-----------------------------------------------------\n";

  // Set the URI to point to the IPAddress of your broker.
  // add any optional params to the url to enable things like
  // tightMarshalling or tcp logging etc.  See the CMS web site for
  // a full list of configuration options.
  //
  //  http://activemq.apache.org/cms/
  //
  // Wire Format Options:
  // =====================
  // Use either stomp or openwire, the default ports are different for each
  //
  // Examples:
  //    tcp://127.0.0.1:61616                      default to openwire
  //    tcp://127.0.0.1:61616?wireFormat=openwire  same as above
  //    tcp://127.0.0.1:61613?wireFormat=stomp     use stomp instead
  //
  std::string brokerURI =
      "failover:(tcp://10.11.1.192:61616"
      "?wireFormat=openwire"
      "&connection.useAsyncSend=true"
      //        "&transport.commandTracingEnabled=true"
      //        "&transport.tcpTracingEnabled=true"
      //        "&wireFormat.tightEncodingEnabled=true"
      ")";

  //============================================================
  // This is the Destination Name and URI options.  Use this to
  // customize where the consumer listens, to have the consumer
  // use a topic or queue set the 'useTopics' flag.
  //============================================================
  std::string destURI =
      "t123"
      "?consumer.prefetchSize = 1 ";  //?consumer.prefetchSize=1";

  //============================================================
  // set to true to use topics instead of queues
  // Note in the code above that this causes createTopic or
  // createQueue to be used in the consumer.
  //============================================================
  bool use_topics = false;

  //============================================================
  // set to true if you want the consumer to use client ack mode
  // instead of the default auto ack mode.
  //============================================================
  bool client_ack = false;

  // Create the consumer
  Consumer consumer(brokerURI, destURI, use_topics, client_ack);

  // Start it up and it will listen forever.
  //  std::thread t1(&SimpleAsyncConsumer::runConsumer, consumer);
  //  t1.join();
  consumer.runConsumer();

  // Wait to exit.
  while (std::cin.get() != 'q') {
  }
  // All CMS resources should be closed before the library is shutdown.
  consumer.close();

  std::cout << "-----------------------------------------------------\n";
  std::cout << "Finished with the example." << std::endl;
  std::cout << "=====================================================\n";
}
int main() {
  activemq::library::ActiveMQCPP::initializeLibrary();
  int thread_count = 12;
  vector<std::thread> threads;
  for (int i = 0; i < thread_count; i++) {
    threads.push_back(std::thread(ClientInitialize));
  }
  for (auto& t : threads) {
    t.join();
  }
  activemq::library::ActiveMQCPP::shutdownLibrary();
  return EXIT_SUCCESS;
}
