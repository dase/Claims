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
 * /Claims/loader/slave_loader.h
 *
 *  Created on: Apr 8, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef LOADER_SLAVE_LOADER_H_
#define LOADER_SLAVE_LOADER_H_
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <string>
#include <atomic>
#include <unordered_map>
#include <fstream>
#include "../catalog/catalog.h"
#include "../storage/BlockManager.h"
#include "../txn_manager/txn.hpp"
#include "../txn_manager/txn_client.hpp"
#include "caf/all.hpp"

namespace claims {
namespace loader {

using std::unordered_map;

using caf::behavior;
using caf::event_based_actor;
using std::string;
using std::fstream;
using claims::catalog::Catalog;
using claims::txn::CheckpointAtom;
using claims::txn::UInt64;
using claims::txn::Query;
using claims::txn::QueryReq;
using claims::txn::GetGlobalPartId;
using claims::txn::TxnClient;
class LoadPacket;

class SlaveLoader {
 public:
  SlaveLoader();
  virtual ~SlaveLoader();

 public:
  static void* StartSlaveLoader(void* arg);

 public:
  RetCode ConnectWithMaster();
  RetCode ReceiveAndWorkLoop();
  void Clean() {
    if (-1 != listening_fd_) FileClose(listening_fd_);
    listening_fd_ = -1;

    if (-1 != master_fd_) FileClose(master_fd_);
    master_fd_ = -1;
  }

 private:
  RetCode EstablishListeningSocket();
  RetCode SendSelfAddrToMaster();
  RetCode GetConnectedSocket();

  void OutputFdIpPort(int fd);

  static RetCode StoreDataInMemory(const LoadPacket& packet);
  static RetCode SendAckToMasterLoader(const uint64_t& txn_id,
                                       bool is_commited);

  static behavior WorkInCAF(event_based_actor* self);
  static behavior PersistInCAF(event_based_actor* self);
  static void WorkInAsync(LoadPacket* packet);
  static void* HandleWork(void* arg);

 private:
  int master_socket_fd_;
  string self_ip;
  int self_port;

  int listening_fd_ = -1;
  int master_fd_ = -1;

 private:
  static caf::actor handle;
  static caf::actor* handles_;
  static caf::actor persistor;

 private:
  queue<LoadPacket*> packet_queue_;
  SpineLock queue_lock_;
  semaphore packet_count_;
  Lock partition_storage_lock_;

 public:
  static ofstream logfile;
};

} /* namespace loader */
} /* namespace claims */

#endif  // LOADER_SLAVE_LOADER_H_
