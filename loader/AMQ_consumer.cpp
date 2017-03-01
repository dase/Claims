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
 * /Claims/loader/AMQ_consumer.cpp
 *
 *  Created on: May 1, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#include "./AMQ_consumer.h"

#include <activemq/core/ActiveMQConnection.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/Session.h>
#include <glog/logging.h>
#include <string>

#include "./master_loader.h"
#include "../common/memory_handle.h"
using activemq::core::ActiveMQConnection;
using activemq::core::ActiveMQConnectionFactory;
using cms::TextMessage;
using std::cout;
using std::endl;
using std::string;

namespace claims {
namespace loader {

void claims::loader::AMQConsumer::run(MasterLoader* mloader) {
  mloader_ = mloader;
  try {
    // Create a ConnectionFactory
    ActiveMQConnectionFactory* connectionFactory =
        new ActiveMQConnectionFactory(brokerURI_);
    LOG(INFO) << "Create a ConnectionFactory";
    // Create a Connection
    connection_ = connectionFactory->createConnection();
    delete connectionFactory;
    LOG(INFO) << "Create a Connection";

    ActiveMQConnection* amqConnection =
        dynamic_cast<ActiveMQConnection*>(connection_);
    if (amqConnection != NULL) {
      amqConnection->addTransportListener(this);
    }
    LOG(INFO) << "Create a ActiveMQConnection";

    connection_->start();

    connection_->setExceptionListener(this);
    LOG(INFO) << "ActiveMQConnection is started";

    // Create a Session
    if (client_ack_) {
      session_ = connection_->createSession(Session::CLIENT_ACKNOWLEDGE);
    } else {
      session_ = connection_->createSession(Session::AUTO_ACKNOWLEDGE);
    }
    LOG(INFO) << "Create a Session";

    // Create the destination (Topic or Queue)
    if (use_topic_) {
      destination_ = session_->createTopic(destURI_);
    } else {
      destination_ = session_->createQueue(destURI_);
    }
    LOG(INFO) << "Create a destination";

    // Create a MessageConsumer from the Session to the Topic or Queue
    consumer = session_->createConsumer(destination_);
    consumer->setMessageListener(this);
    LOG(INFO) << "Create a MessageConsumer";
    LOG(INFO) << " ready to receive ingest message from ActiveMQ";
    std::cout << "AMQ client listening...." << std::endl;
  } catch (CMSException& e) {
    e.printStackTrace();
  }
}

void claims::loader::AMQConsumer::onMessage(const Message* message) {
  try {
    const TextMessage* textMessage = dynamic_cast<const TextMessage*>(message);
    string text = "";
    mloader_->AddCount();
    if (textMessage != NULL) {
      text = textMessage->getText();
      //  message->acknowledge();
      //  cout << "<packet>" << endl << text << endl;
      bool client_ack = client_ack_;
      mloader_->Ingest(text, [message, client_ack]() -> int {
        if (client_ack) {
          message->acknowledge();
        }
        return claims::common::rSuccess;
      });
    } else {
      text = "NOT A TEXTMESSAGE!";
      std::cerr << "get error message from AMQ " << std::endl;
      LOG(ERROR) << "get error message from AMQ ";
    }
    // enter for load.
    //      printf("Message #%d Received: %s\n", count, text.c_str());
  } catch (CMSException& e) {
    e.printStackTrace();
  }
}

void claims::loader::AMQConsumer::cleanup() {
  //*************************************************
  // Always close destination, consumers and producers before
  // you destroy their sessions and connection.
  //*************************************************

  // Destroy resources.
  try {
    DELETE_PTR(destination_);
  } catch (CMSException& e) {
  }

  try {
    DELETE_PTR(consumer);
  } catch (CMSException& e) {
  }

  // Close open resources.
  try {
    if (session_ != NULL) session_->close();
    if (connection_ != NULL) connection_->close();
  } catch (CMSException& e) {
  }

  // Now Destroy them
  try {
    DELETE_PTR(session_);
  } catch (CMSException& e) {
  }

  try {
    DELETE_PTR(connection_);
  } catch (CMSException& e) {
  }
}

} /* namespace loader */
} /* namespace claims */
