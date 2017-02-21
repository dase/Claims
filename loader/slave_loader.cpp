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
 * /Claims/loader/slave_loader.cpp
 *
 *  Created on: Apr 8, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#include "./slave_loader.h"

#include <glog/logging.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>
#include <future>  //NOLINT
#include <chrono>  //NOLINT
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "./load_packet.h"
#include "../Config.h"
#include "../Environment.h"
#include "../common/error_define.h"
#include "../common/ids.h"
#include "../common/memory_handle.h"
#include "../storage/BlockManager.h"
#include "../storage/ChunkStorage.h"
#include "../storage/PartitionStorage.h"
#include "../txn_manager/txn.hpp"
#include "../utility/resource_guard.h"

#include "../node_manager/base_node.h"

using caf::behavior;
using caf::event_based_actor;
using caf::io::remote_actor;
using caf::spawn;
using claims::common::Malloc;
using claims::common::rSuccess;
using claims::common::rFailure;
using claims::txn::GetPartitionIdFromGlobalPartId;
using claims::txn::GetProjectionIdFromGlobalPartId;
using claims::txn::GetTableIdFromGlobalPartId;
using claims::MasterNode;
using std::chrono::milliseconds;
using std::chrono::seconds;

#define WORK_THREADS
// #define ONE_WORK_THREAD

// #define NO_ACTUAL_WORK

// #define SLAVE_LOADER_PREF

// #define MASTER_LOADER_PREF

#ifdef MASTER_LOADER_PREF
#define PERFLOG(info) LOG(INFO) << info << endl;
#else
#define PERFLOG
#endif

#ifdef SLAVE_LOADER_PREF
#define ATOMIC_ADD(var, value) __sync_add_and_fetch(&var, value);
#define GET_TIME_SL(var) GETCURRENTTIME(var);
#else
#define ATOMIC_ADD(var, value)
#define GET_TIME_SL(var)
#endif

caf::actor SlaveLoader::handle;
caf::actor* SlaveLoader::handles_;
caf::actor SlaveLoader::persistor;

static const int txn_count_for_debug = 5000;
static const char* txn_count_string = "5000";

namespace claims {
namespace loader {

ofstream SlaveLoader::logfile;

SlaveLoader::SlaveLoader() {}
SlaveLoader::~SlaveLoader() {}

RetCode SlaveLoader::ConnectWithMaster() {
  int ret = rSuccess;
  int retry_time = 10;
  for (int i = 0; Clean(), i < retry_time; ++i) {  // if failed, call Clean()
    EXEC_AND_LOG(ret, EstablishListeningSocket(),
                 "established listening socket",
                 "failed to establish listening socket in " << i << " times");
    if (rSuccess == ret) break;
  }
  if (rSuccess != ret) {
    Clean();
    return ret;
  }

  for (int i = 1; i <= retry_time; ++i) {
    EXEC_AND_LOG(ret, SendSelfAddrToMaster(), "sent self ip/port to master",
                 "failed to send self ip/port to master in " << i << " times");
    if (rSuccess == ret) break;
    sleep(1);
  }
  if (rSuccess != ret) {
    Clean();
    return ret;
  }

  for (int i = 0; i < retry_time; ++i) {
    EXEC_AND_LOG(ret, GetConnectedSocket(), "got connected socket with master",
                 "failed to get connected socket with master in " << i
                                                                  << " times");
    if (rSuccess == ret) break;
  }
  if (rSuccess != ret) Clean();
  return ret;
}

RetCode SlaveLoader::EstablishListeningSocket() {
  int ret = rSuccess;
  listening_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == listening_fd_) {
    PLOG(ERROR) << "failed to create socket";
  }

  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = 0;
  sock_addr.sin_addr.s_addr =
      inet_addr(Environment::getInstance()->getIp().c_str());

  if (-1 ==
      bind(listening_fd_, (struct sockaddr*)(&sock_addr), sizeof(sock_addr))) {
    PLOG(ERROR) << "failed to bind socket";
    return rFailure;
  }

  OutputFdIpPort(listening_fd_);

  if (-1 == listen(listening_fd_, 5)) {
    PLOG(ERROR) << "failed to listen socket";
    return rFailure;
  }

  OutputFdIpPort(listening_fd_);

  struct sockaddr_in temp_addr;
  socklen_t addr_len = sizeof(sockaddr_in);
  if (-1 ==
      getsockname(listening_fd_, (struct sockaddr*)(&temp_addr), &addr_len)) {
    PLOG(ERROR) << "failed to get socket name ";
    return rFailure;
  }

  self_port = ntohs(temp_addr.sin_port);
  self_ip = inet_ntoa(temp_addr.sin_addr);

  LOG(INFO) << "slave socket IP:" << self_ip << ", port:" << self_port;
  return ret;
}

RetCode SlaveLoader::SendSelfAddrToMaster() {
  //  auto send_actor = spawn([&](event_based_actor* self) {
  //    auto master_actor =
  //        remote_actor(Config::master_loader_ip,
  //        Config::master_loader_port);
  //    self->sync_send(master_actor, IpPortAtom::value, self_ip, self_port);
  //  });
  LOG(INFO) << "going to send self (" << self_ip << ":" << self_port << ")"
            << "to (" << Config::master_loader_ip << ":"
            << Config::master_loader_port << ")";
  try {
    static auto master_actor =
        remote_actor(Config::master_loader_ip, Config::master_loader_port);
    caf::scoped_actor self;
    self->sync_send(master_actor, IpPortAtom::value, self_ip, self_port)
        .await([&](int r) {  // NOLINT
          LOG(INFO) << "sent ip&port and received response";
        });
  } catch (exception& e) {
    LOG(ERROR) << "can't send self ip&port to master loader. " << e.what();
    return rFailure;
  }
  return rSuccess;
}

RetCode SlaveLoader::GetConnectedSocket() {
  assert(listening_fd_ > 3);
  OutputFdIpPort(listening_fd_);
  LOG(INFO) << "fd is accepting...";

  struct sockaddr_in master_addr;
  socklen_t len = sizeof(sockaddr_in);
  int master_fd = accept(listening_fd_, (struct sockaddr*)(&master_addr), &len);
  if (-1 == master_fd) {
    PLOG(ERROR) << "failed to accept socket";
    return rFailure;
  }
  master_fd_ = master_fd;
  return rSuccess;
}
void SlaveLoader::OutputFdIpPort(int fd) {
  struct sockaddr_in temp_addr;
  socklen_t addr_len = sizeof(sockaddr_in);
  if (-1 == getsockname(fd, (struct sockaddr*)(&temp_addr), &addr_len)) {
    PLOG(ERROR) << "failed to get socket name ";
  }
  LOG(INFO) << "fd ----> (" << inet_ntoa(temp_addr.sin_addr) << ":"
            << ntohs(temp_addr.sin_port) << ")";
}

RetCode SlaveLoader::ReceiveAndWorkLoop() {
  assert(master_fd_ > 3);
  static uint64_t total_handle_time = 0;
  static int handle_count = 0;
  char head_buffer[LoadPacket::kHeadLength];
  int real_read_num;
  LOG(INFO) << "slave is receiving ...";
  while (1) {
    RetCode ret = rSuccess;

    /// get load packet
    if (-1 == (real_read_num = recv(master_fd_, head_buffer,
                                    LoadPacket::kHeadLength, MSG_WAITALL))) {
      PLOG(ERROR) << "failed to receive message length from master";
      return rFailure;
    } else if (0 == real_read_num) {
      PLOG(ERROR) << "master loader socket has been closed";
      return rFailure;
    } else if (real_read_num < LoadPacket::kHeadLength) {
      LOG(ERROR) << "received message error! only read " << real_read_num
                 << " bytes";
      continue;
    }
    GET_TIME_SL(start_handle);
    PERFLOG("received packet head");
    uint64_t data_length =
        *reinterpret_cast<uint64_t*>(head_buffer + LoadPacket::kHeadLength -
                                     sizeof(uint64_t));
    uint64_t real_packet_length = data_length + LoadPacket::kHeadLength;
    DLOG(INFO) << "real packet length is :" << real_packet_length
               << ". date length is " << data_length;
    assert(data_length >= 4 && data_length <= 10000000);
    char* data_buffer = Malloc(data_length);
    if (NULL == data_buffer) {
      ELOG((ret = claims::common::rNoMemory),
           "no memory to hold data of message from master");
      return ret;
    }

    if (-1 == recv(master_fd_, data_buffer, data_length, MSG_WAITALL)) {
      PLOG(ERROR) << "failed to receive message from master";
      return claims::common::rReceiveMessageError;
    }
    //    LOG(INFO) << "data of message from master is:" << buffer;

    /// deserialization of packet
    PERFLOG("got all packet buffer");
#ifdef WORK_THREADS
    LoadPacket* packet = new LoadPacket();
    EXEC_AND_DLOG(ret, packet->Deserialize(head_buffer, data_buffer),
                  "deserialized packet", "failed to deserialize packet");

    static int handle_index = 0;
    caf::scoped_actor self;
    self->send(handles_[handle_index], LoadPacketAtom::value, packet);
    if (++handle_index == Config::slave_loader_thread_num) handle_index = 0;

#else
#ifdef ONE_WORK_THREAD
    LoadPacket* packet = new LoadPacket();
    EXEC_AND_DLOG(ret, packet->Deserialize(head_buffer, data_buffer),
                  "deserialized packet", "failed to deserialize packet");
    //    std::async(std::launch::async, WorkInAsync, packet);  // too slow
    caf::scoped_actor self;
    self->send(handle, LoadPacketAtom::value, packet);
#else
    LoadPacket packet;
#ifndef NO_ACTUAL_WORK
    EXEC_AND_DLOG(ret, packet.Deserialize(head_buffer, data_buffer),
                  "deserialized packet", "failed to deserialize packet");

    EXEC_AND_DLOG(ret, StoreDataInMemory(packet), "stored data",
                  "failed to store");
#else
    packet.txn_id_ = *reinterpret_cast<const uint64_t*>(head_buffer);
#endif
    /// return result to master loader
    EXEC_AND_LOG(ret, SendAckToMasterLoader(packet.txn_id_, rSuccess == ret),
                 "sent commit result of " << packet.txn_id_
                                          << " to master loader",
                 "failed to send commit res to master loader");
#endif
#endif

#ifdef SLAVE_LOADER_PREF
    ATOMIC_ADD(total_handle_time, GetElapsedTimeInUs(start_handle));
    if (txn_count_for_debug == ++handle_count) {
      cout << "handle " << handle_count
           << " messages used:" << total_handle_time << endl;
    }
#endif
    if (rSuccess != ret) return ret;
  }
}

RetCode SlaveLoader::StoreDataInMemory(const LoadPacket& packet) {
  RetCode ret = rSuccess;
  const uint64_t table_id = GetTableIdFromGlobalPartId(packet.global_part_id_);
  const uint64_t prj_id =
      GetProjectionIdFromGlobalPartId(packet.global_part_id_);
  const uint64_t part_id =
      GetPartitionIdFromGlobalPartId(packet.global_part_id_);

  PartitionStorage* part_storage =
      BlockManager::getInstance()->GetPartitionHandle(
          PartitionID(ProjectionID(table_id, prj_id), part_id));
  assert(part_storage != NULL);

  /// set HDFS because the memory is not applied actually
  /// it will be set to MEMORY in function
  uint64_t last_chunk_id = (packet.pos_ + packet.offset_) / CHUNK_SIZE;
  //  assert(last_chunk_id <=
  //             (1024UL * 1024 * 1024 * 1024 * 1024) / (64 * 1024 * 1024) &&
  //         " memory for chunk should not larger than 1PB");
  DLOG(INFO) << "position+offset is:" << packet.pos_ + packet.offset_
             << " CHUNK SIZE is:" << CHUNK_SIZE
             << " last chunk id is:" << last_chunk_id;
  EXEC_AND_DLOG_RETURN(
      ret, part_storage->AddRtChunkWithMemoryApply(last_chunk_id + 1, MEMORY),
      "added chunk to " << last_chunk_id + 1, "failed to add chunk");
  // cout << "******1*****" << endl;
  // copy data into applied memory
  Schema* schema = Catalog::getInstance()
                       ->getTable(table_id)
                       ->getProjectoin(prj_id)
                       ->getSchema();
  MemoryGuard<Schema> schema_guard(schema);
  const uint64_t tuple_size = schema->getTupleMaxSize();

  uint64_t cur_chunk_id = packet.pos_ / CHUNK_SIZE;
  uint64_t cur_block_id = (packet.pos_ % CHUNK_SIZE) / BLOCK_SIZE;
  uint64_t pos_in_block = packet.pos_ % BLOCK_SIZE;
  uint64_t total_written_length = 0;
  uint64_t data_length = packet.data_length_;
  HdfsInMemoryChunk chunk_info;

  /*  for (auto p = 0; p < data_length; p += tuple_size) {
      for (auto col = 1; col < schema->getncolumns(); col++) {
        logfile << schema->getColumnValue(packet.data_buffer_ + p, col) + "|";
      }
      logfile << endl;
    }*/

  while (total_written_length < data_length) {
    /// get start position of current chunk
    if (BlockManager::getInstance()->getMemoryChunkStore()->GetChunk(
            ChunkID(PartitionID(ProjectionID(table_id, prj_id), part_id),
                    cur_chunk_id, true),
            chunk_info)) {
      // In this version, the last chunk info don't updated their member: length
      // after inserting data,
      // so the data inserted into last chunk is invisible.
      // This leads to no new data is show in the beginning of loading.
      // The new applied chunk has updated its length to CHUNK_SIZE,
      // so they performs well
      DLOG(INFO) << "start address of chunk:" << cur_chunk_id << " is "
                 << chunk_info.hook << ", end addr is "
                 << chunk_info.hook + CHUNK_SIZE;
      InMemoryChunkWriterIterator writer(chunk_info.hook, CHUNK_SIZE,
                                         cur_block_id, BLOCK_SIZE, pos_in_block,
                                         tuple_size, schema);
      // cout << "store data length:" << data_length << endl;
      do {  // write to every block
        /*        logfile << cur_chunk_id << "->" << writer.GetBlockId() << "->"
                        << writer.GetBlockPos() << ",";*/
        uint64_t written_length =
            writer.Write(packet.data_buffer_ + total_written_length,
                         data_length - total_written_length);
        /*        logfile << writer.GetBlockPos() + written_length << ","
                        << written_length / schema->getTupleMaxSize() << endl;*/
        total_written_length += written_length;
        DLOG(INFO) << "written " << written_length
                   << " bytes into chunk:" << cur_chunk_id
                   << ". Now total written " << total_written_length << " bytes"
                   << endl;
        if (total_written_length == data_length) {
          // all tuple is written into memory
          return rSuccess;
        } else if (total_written_length > data_length) {
          assert(false);
        }
      } while (writer.NextBlock());
      ++cur_chunk_id;  // get next chunk to write
      DLOG(INFO) << "Now chunk id is " << cur_chunk_id
                 << ", total number of chunk is"
                 << part_storage->GetRTChunkNum();
      if (cur_chunk_id < part_storage->GetRTChunkNum()) {
        assert(cur_chunk_id < part_storage->GetRTChunkNum() && cur_chunk_id);
      }
      cur_block_id = 0;  // the block id of next chunk is 0
      pos_in_block = 0;
    } else {
      LOG(INFO) << "chunk id is " << cur_chunk_id << endl;
      cout << "get chunk:" << cur_chunk_id << " failed" << endl;
      assert(false && "no chunk with this chunk id");
    }
  }
  delete schema;
  return ret;
}

RetCode SlaveLoader::SendAckToMasterLoader(const uint64_t& txn_id,
                                           bool is_commited) {
  static uint64_t total_send_time = 0;
  static uint64_t count = 0;
  GET_TIME_SL(send_start);
  try {
    static auto master_actor =
        remote_actor(Config::master_loader_ip, Config::master_loader_port);
    static caf::scoped_actor self;
    self->send(master_actor, LoadAckAtom::value, txn_id, is_commited);
#ifdef SLAVE_LOADER_PREF
    ATOMIC_ADD(total_send_time, GetElapsedTimeInUs(send_start));
    if (txn_count_for_debug == ++count) {
      cout << "send " << count << " ACK used:" << total_send_time
           << " us. average time is:" << total_send_time / count << endl;
    }
#endif
    return rSuccess;
  } catch (exception& e) {
    LOG(ERROR) << "failed to send commit result of " << txn_id
               << " to master loader ." << e.what();
    return rFailure;
  }
}

// this method has the best performance
behavior SlaveLoader::WorkInCAF(event_based_actor* self) {
  remove("slave_loader_log.txt");
  logfile.open("slave_loader_log.txt");
  return {[=](LoadPacketAtom, LoadPacket* packet) {  // NOLINT
    RetCode ret = rSuccess;

    EXEC_AND_DLOG(ret, StoreDataInMemory(*packet), "stored data",
                  "failed to store");
    /// return result to master loader
    EXEC_AND_LOG(ret, SendAckToMasterLoader(packet->txn_id_, rSuccess == ret),
                 "sent commit result of " << packet->txn_id_
                                          << " to master loader",
                 "failed to send commit res to master loader");
    DELETE_PTR(packet->data_buffer_);
    DELETE_PTR(packet);
  }};
}

behavior SlaveLoader::PersistInCAF(event_based_actor* self) {
  self->delayed_send(self,
                     seconds(30 + Environment::getInstance()->getNodeID() * 2),
                     CheckpointAtom::value);
  return {[self](CheckpointAtom) {
    QueryReq query_req;
    query_req.include_abort_ = true;
    Query query;
    auto part_list =
        Environment::getInstance()->get_block_manager()->GetAllPartition();
    for (auto& part : part_list)
      query_req.part_list_.push_back(GetGlobalPartId(part));
    TxnClient::BeginQuery(query_req, query);
    for (auto& part : part_list) {
      UInt64 g_part_id = GetGlobalPartId(part);
      if (query.snapshot_[g_part_id].size() > 0) {
        auto part_handler =
            Environment::getInstance()->get_block_manager()->GetPartitionHandle(
                part);
        auto new_rt_cp = query.snapshot_[g_part_id].rbegin()->first +
                         query.snapshot_[g_part_id].rbegin()->second;
        /*for debug new_rt_cp = query.snapshot_[g_part_id].begin()->first +
                                  query.snapshot_[g_part_id].begin()->second;*/
        /* for debug vector<PStrip> ch_list = {
            {query.snapshot_[g_part_id].begin()->first,
             query.snapshot_[g_part_id].begin()->second}}; */
        auto old_rt_cp = query.snapshot_[g_part_id].begin()->first;
        // merge from historical to real time
        auto old_his_cp = query.his_cp_list_[g_part_id];
        auto new_his_cp = part_handler->MergeToHis(
            old_his_cp, /*ch_list */ query.snapshot_[g_part_id]);
        cout << "merge his:<" << old_his_cp / BLOCK_SIZE << ","
             << old_his_cp % BLOCK_SIZE << "> => <" << new_his_cp / BLOCK_SIZE
             << "," << new_his_cp % BLOCK_SIZE << ">" << endl;
        cout << "merge rt:<" << old_rt_cp / BLOCK_SIZE << ","
             << old_rt_cp % BLOCK_SIZE << "> => <" << new_rt_cp / BLOCK_SIZE
             << "," << new_rt_cp % BLOCK_SIZE << ">" << endl;
        // cout << "new_his_cp:" << new_his_cp << endl;
        if (new_his_cp == old_his_cp) {
          cout << "don't need to create checkpoint" << endl;
          continue;
        }
        RetCode ret = rSuccess;
        if (ret == rSuccess)
          ret = part_handler->Persist(old_his_cp, new_his_cp);
        else
          continue;
        if (ret == rSuccess)
          ret = TxnClient::CommitCheckpoint(query.ts_, g_part_id, new_his_cp,
                                            new_rt_cp);
        else
          continue;
        if (ret == rSuccess) {
          auto slave_node = Environment::getInstance()->get_slave_node();
          ret = slave_node->AddBlocks(g_part_id,
                                      (new_his_cp - old_his_cp) / BLOCK_SIZE);
          /*         auto cata = Catalog::getInstance();
                   auto proj = cata->getProjection(part.projection_id);
                   proj->getPartitioner()->addPartitionBlocks(
                       part.partition_off, (new_his_cp - old_his_cp) /
             BLOCK_SIZE);
                   auto schema = proj->getSchema();
                   MemoryGuard<Schema> schema_guard(schema);
                   auto tuple_size = schema->getTupleMaxSize();
                   proj->getPartitioner()->addPartitionCardinality(
                       part.partition_off, (new_rt_cp - old_rt_cp) /
             tuple_size);
                   ret = cata->saveCatalog();*/
        }
        if (ret == rSuccess)
          cout << "Persist success" << endl;
        else
          cout << "Persist fail" << endl;
      }
    }
    self->delayed_send(self, seconds(30), CheckpointAtom::value);
  }};
}

// It's very slow
void SlaveLoader::WorkInAsync(LoadPacket* packet) {
  RetCode ret = rSuccess;
  EXEC_AND_DLOG(ret, StoreDataInMemory(*packet), "stored data",
                "failed to store");

  /// return result to master loader
  EXEC_AND_LOG(ret, SendAckToMasterLoader(packet->txn_id_, rSuccess == ret),
               "sent commit result of " << packet->txn_id_
                                        << " to master loader",
               "failed to send commit res to master loader");
  DELETE_PTR(packet);
}

// it is so slow to deliver message using queue
void* SlaveLoader::HandleWork(void* arg) {
  SlaveLoader* slave_loader = static_cast<SlaveLoader*>(arg);
  while (1) {
    RetCode ret = rSuccess;
    slave_loader->packet_count_.wait();
    LoadPacket* packet = nullptr;
    {
      LockGuard<SpineLock> guard(slave_loader->queue_lock_);
      packet = slave_loader->packet_queue_.front();
      slave_loader->packet_queue_.pop();
    }

    EXEC_AND_DLOG(ret, SlaveLoader::StoreDataInMemory(*packet), "stored data",
                  "failed to store");

    /// return result to master loader
    EXEC_AND_LOG(
        ret,
        SlaveLoader::SendAckToMasterLoader(packet->txn_id_, rSuccess == ret),
        "sent commit result of " << packet->txn_id_ << " to master loader",
        "failed to send commit res to master loader");
    DELETE_PTR(packet);
  }
}

void* SlaveLoader::StartSlaveLoader(void* arg) {
  Config::getInstance();
  LOG(INFO) << "start slave loader...";
  sleep(2);

  SlaveLoader* slave_loader = Environment::getInstance()->get_slave_loader();
  int ret = rSuccess;
  EXEC_AND_DLOG(ret, slave_loader->ConnectWithMaster(),
                "succeed to connect with master",
                "failed to connect with master ");

  assert(rSuccess == ret && "can't connect with master");

  cout << "connected with master loader" << endl;
// TODO(YK): error handle

#ifdef ONE_WORK_THREAD
  SlaveLoader::handle = caf::spawn(SlaveLoader::WorkInCAF);
#endif

#ifdef WORK_THREADS
  //  for (int i = 0; i < 1; ++i) {
  //    Environment::getInstance()->getThreadPool()->AddTask(
  //        SlaveLoader::HandleWork, slave_loader);
  //  }

  handles_ = new caf::actor[Config::slave_loader_thread_num];

  for (int i = 0; i < Config::slave_loader_thread_num; ++i) {
    SlaveLoader::handles_[i] = caf::spawn(SlaveLoader::WorkInCAF);
  }
#endif
  persistor = caf::spawn(SlaveLoader::PersistInCAF);

  slave_loader->ReceiveAndWorkLoop();
  assert(false);
  return NULL;
}

} /* namespace loader */
} /* namespace claims */
