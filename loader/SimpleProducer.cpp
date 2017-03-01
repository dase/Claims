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
 * /ActiveMQ_c/SimpleProducer.cpp
 *
 *  Created on: 2016年4月9日
 *      Author: Han
 *		   Email:
 *
 * Description:
 *
 */

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Long.h>
#include <decaf/util/Date.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <sys/time.h>
#include <thread>

using namespace activemq;
using namespace activemq::core;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
class Producer : public Runnable {
 private:
  Connection* connection_;
  Session* session_;
  Destination* destination_;
  MessageProducer* producer_;
  bool use_topic_;
  bool client_ack_;
  unsigned int num_messages_;
  std::string brokerURI_;
  std::string destURI_;
  std::string file_path_;
  std::string table_name_;

 private:
  Producer(const Producer&);
  Producer& operator=(const Producer&);

 public:
  Producer(const std::string& brokerURI, unsigned int num_messages,
           const std::string& destURI, const std::string& file_path,
           const std::string& table_name, bool use_topic = false,
           bool client_ack = false)
      : connection_(NULL),
        session_(NULL),
        destination_(NULL),
        producer_(NULL),
        use_topic_(use_topic),
        client_ack_(client_ack),
        num_messages_(num_messages),
        brokerURI_(brokerURI),
        destURI_(destURI),
        file_path_(file_path),
        table_name_(table_name) {}

  virtual ~Producer() { cleanup(); }

  string GetText(ifstream& file1, const string& row_dsp) {
    int count = 0;
    string c = "";
    size_t cur_p = 0;
    if (file1.is_open()) {
      char* str1 = (char*)malloc(1000);
      if (1 == row_dsp.length()) {
        do {
          file1.getline(str1, 1000, (char)row_dsp[0]);
          if (file1.eof()) break;
          //          cout << str1 << endl;
          long int length = strlen(str1);
          //          cout << "the length is " << length << endl;
          c += str1;
          c += row_dsp;
          count++;
        } while (count < 150);
      }
      free(str1);
    }
    return c;
  }
  void SendMessage(string filename) {
    cout << "SendMessage thread is created" << endl;
    /*
        ifstream file1;
        file1.open(filename.c_str());
        for (unsigned int ix = 0; ix < num_messages_; ++ix) {
          if (file1.eof()) break;
          string title = table_name_ + ",|,\n,";
          string body = GetText(file1, "\n");
          string text = title + body;
          //      cout << text << endl;
          TextMessage* message = session_->createTextMessage(text);
          message->setIntProperty("Integer", ix);
          producer_->send(message);
          //      std::cout << "sent a message" << std::endl;
          delete message;
        }
        file1.close();

        std::cout << "sent all message" << std::endl;
    */

    int loop_count, row_count, index;
    cout << "Input the row_id start:";
    cin >> index;
    auto gen_rows = [&index](int count) {
      string rows = "";
      for (auto i = 0; i < count; i++)
        rows += std::to_string(index) + "|" + std::to_string(i) + "\n";
      index++;
      return rows;
    };
    string mode;
    while (true) {
      cout << "Input the loop-count row-count:";
      cin >> loop_count >> row_count;
      ofstream outfile("genlog.txt");
      for (auto i = 0; i < loop_count; i++) {
        string title = "test,|,\n,";
        string body = gen_rows(row_count);
        TextMessage* message = session_->createTextMessage(title + body);
        // message->setIntProperty("Integer", index);
        producer_->send(message);
        outfile << body;
        delete message;
      }
      outfile.close();
    }
  }
  void close() { this->cleanup(); }

  virtual void run() {
    try {
      // Create a ConnectionFactory
      auto_ptr<ActiveMQConnectionFactory> connectionFactory(
          new ActiveMQConnectionFactory(brokerURI_));
      cout << "Create a ConnectionFactory" << endl;
      // Create a Connection
      try {
        connection_ = connectionFactory->createConnection();
        connection_->start();
      } catch (CMSException& e) {
        e.printStackTrace();
        throw e;
      }
      cout << "Create a Connection" << endl;

      // Create a Session
      if (client_ack_) {
        session_ = connection_->createSession(Session::CLIENT_ACKNOWLEDGE);
      } else {
        session_ = connection_->createSession(Session::AUTO_ACKNOWLEDGE);
      }
      cout << "Create a Session" << endl;

      // Create the destination (Topic or Queue)
      if (use_topic_) {
        destination_ = session_->createTopic(destURI_);
      } else {
        destination_ = session_->createQueue(destURI_);
      }
      cout << "Create the destination (Topic or Queue)" << endl;

      // Create a MessageProducer from the Session to the Topic or Queue
      producer_ = session_->createProducer(destination_);
      producer_->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
      cout << "Create a MessageProducer" << endl;

      timeval start;
      gettimeofday(&start, NULL);
      cout << "going to create threads" << endl;
      vector<std::thread> threads;
      // int thread_count = 8;
      int thread_count = 1;
      for (int i = 1; i <= thread_count; i++) {
        //        string tmp = filename + (char)(i + 48);
        cout << file_path_ << endl;
        threads.push_back(
            std::thread(&Producer::SendMessage, this, file_path_));
      }
      for (auto& t : threads) {
        t.join();
      }

      timeval end;
      gettimeofday(&end, NULL);
      cout << "send message time :"
           << (end.tv_usec - start.tv_usec) / 1000.0 +
                  (end.tv_sec - start.tv_sec) * 1000.0;

    } catch (CMSException& e) {
      e.printStackTrace();
    }
  }

 private:
  void cleanup() {
    // Destroy resources.
    try {
      if (destination_ != NULL) delete destination_;
    } catch (CMSException& e) {
      e.printStackTrace();
    }
    destination_ = NULL;

    try {
      if (producer_ != NULL) delete producer_;
    } catch (CMSException& e) {
      e.printStackTrace();
    }
    producer_ = NULL;

    // Close open resources.
    try {
      if (session_ != NULL) session_->close();
      if (connection_ != NULL) connection_->close();
    } catch (CMSException& e) {
      e.printStackTrace();
    }

    try {
      if (session_ != NULL) delete session_;
    } catch (CMSException& e) {
      e.printStackTrace();
    }
    session_ = NULL;

    try {
      if (connection_ != NULL) delete connection_;
    } catch (CMSException& e) {
      e.printStackTrace();
    }
    connection_ = NULL;
  }
};

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  //  getopt();/
  if (argc < 3) perror("error");

  std::string file_path = argv[1];  // first parameter

  std::string table_name = argv[2];  // table name

  activemq::library::ActiveMQCPP::initializeLibrary();

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
      "failover://(tcp://58.198.176.92:61616"  //"failover://(tcp://58.198.176.92:61616"
      "?wireFormat=openwire"
      "&connection.useAsyncSend=true"
      //        "&transport.commandTracingEnabled=true"
      //        "&transport.tcpTracingEnabled=true"
      //        "&wireFormat.tightEncodingEnabled=true"
      ")";

  //============================================================
  // Total number of messages for this producer to send.
  //============================================================
  unsigned int num_messages = 200000;

  //============================================================
  // This is the Destination Name and URI options.  Use this to
  // customize where the Producer produces, to have the producer
  // use a topic or queue set the 'useTopics' flag.
  //============================================================
  std::string destURI = "t123";

  //============================================================
  // set to true to use topics instead of queues
  // Note in the code above that this causes createTopic or
  // createQueue to be used in the producer.
  //============================================================
  bool use_topics = false;

  bool client_ack = false;

  cout << "new producer" << endl;
  // Create the producer_ and run it.
  Producer producer(brokerURI, num_messages, destURI, file_path, table_name,
                    use_topics, client_ack);

  cout << "start run producer" << endl;
  // Publish the given number of Messages
  producer.run();

  // Before exiting we ensure that all CMS resources are closed.
  producer.close();

  std::cout << "-----------------------------------------------------\n";
  std::cout << "Finished with the example." << std::endl;
  std::cout << "=====================================================\n";

  activemq::library::ActiveMQCPP::shutdownLibrary();
  return EXIT_SUCCESS;
}
