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
 * /Claims/loader/master_loader.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#include "./master_loader.h"

#include <activemq/library/ActiveMQCPP.h>
#include <glog/logging.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <queue>
#include <utility>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "./AMQ_consumer.h"
#include "./load_packet.h"
#include "./loader_message.h"
#include "./validity.h"
#include "../catalog/catalog.h"
#include "../catalog/partitioner.h"
#include "../catalog/table.h"
#include "../common/data_type.h"
#include "../common/error_define.h"
#include "../common/ids.h"
#include "../common/memory_handle.h"
#include "../common/Schema/TupleConvertor.h"
#include "../Config.h"
#include "../Environment.h"
#include "../loader/data_ingestion.h"
#include "../Resource/NodeTracker.h"
#include "../txn_manager/txn.hpp"
#include "../txn_manager/txn_client.hpp"
#include "../txn_manager/txn_log.hpp"
#include "../utility/cpu_scheduler.h"
#include "../utility/resource_guard.h"
#include "../utility/Timer.h"
using caf::aout;
using caf::behavior;
using caf::event_based_actor;
using caf::io::publish;
using caf::io::remote_actor;
using caf::spawn;
using std::endl;
using claims::catalog::Catalog;
using claims::catalog::Partitioner;
using claims::catalog::TableDescriptor;
using claims::common::Malloc;
using claims::common::rSuccess;
using claims::common::rFailure;
using namespace claims::txn;  // NOLINT

// #define SEND_THREAD

// #define NON_BLOCK_SOCKET

#define MASTER_LOADER_PREF
// #define MASTER_LOADER_DEBUG

#ifdef MASTER_LOADER_DEBUG
#define PERFLOG(info) LOG(INFO) << info << endl;
#else
#define PERFLOG(info)
#endif

#ifdef MASTER_LOADER_PREF
#define ATOMIC_ADD(var, value) __sync_add_and_fetch(&var, value);
#define GET_TIME_ML(var) GETCURRENTTIME(var);
#else
#define ATOMIC_ADD(var, value)
#define GET_TIME_ML(var)
#endif

uint64_t MasterLoader::debug_finished_txn_count = 0;
uint64_t MasterLoader::debug_consumed_message_count = 0;
timeval MasterLoader::start_time;
uint64_t MasterLoader::txn_average_delay_ = 0;
static int MasterLoader::buffer_full_time = 0;
atomic<uint64_t> message_count;
static const int txn_count_for_debug = 10000;

namespace claims {
namespace loader {

ofstream MasterLoader::logfile;

void MasterLoader::AddCount() {
  message_count.fetch_add(1);
  // cout << "new message" << endl;
}

behavior MasterLoader::Monitor(event_based_actor* self) {
  static uint64_t last_count = 0;
  self->delayed_send(self, seconds(5), "monitor");
  return {[self](const string& str) {
    auto tmp = message_count.load();
    if (tmp != last_count) {
      last_count = tmp;
      cout << "master_loader receive count==>" << last_count << endl;
    }
    self->delayed_send(self, seconds(5), "monitor");
  }};
}

void MasterLoader::IngestionRequest::Show() {
  DLOG(INFO) << "table name:" << table_name_
             << ", column separator:" << col_sep_
             << ", row separator:" << row_sep_
             << ", tuples size is:" << tuples_.size();
}

MasterLoader::MasterLoader()
    : master_loader_ip_(Config::master_loader_ip),
      master_loader_port_(Config::master_loader_port),
      send_thread_num_(Config::master_loader_thread_num / 2 + 1) {
#ifdef SEND_THREAD
  packet_queues_ = new queue<LoadPacket*>[send_thread_num_];
  packet_queue_lock_ = new SpineLock[send_thread_num_];
  packet_queue_to_send_count_ = new semaphore[send_thread_num_];
#endif
}

MasterLoader::~MasterLoader() {
#ifdef SEND_THREAD
  for (int i = 0; i < send_thread_num_; ++i) {
    while (!packet_queues_[i].empty()) {
      DELETE_PTR(packet_queues_[i].front());
      packet_queues_[i].pop();
    }
  }
#endif
  DELETE_ARRAY(packet_queues_);
  DELETE_ARRAY(packet_queue_lock_);
  DELETE_ARRAY(packet_queue_to_send_count_);
}

static behavior MasterLoader::ReceiveSlaveReg(event_based_actor* self,
                                              MasterLoader* mloader) {
  return {
      [=](IpPortAtom, std::string ip, int port) -> int {  // NOLINT
        LOG(INFO) << "receive slave network address(" << ip << ":" << port
                  << ")" << endl;
        int new_slave_fd = -1;
        if (rSuccess !=
            mloader->GetSocketFdConnectedWithSlave(ip, port, &new_slave_fd)) {
          LOG(ERROR) << "failed to get connected fd with slave";
        } else {
          LOG(INFO) << "succeed to get connected fd " << new_slave_fd
                    << "with slave";
        }
        assert(new_slave_fd > 3);

        DLOG(INFO) << "going to push socket into map";
        mloader->slave_addr_to_socket_[NodeAddress(ip, "")] = new_slave_fd;
        mloader->socket_fd_to_lock_[new_slave_fd] = Lock();
        DLOG(INFO) << "start to send test message to slave";

        cout << "slave_node <=> socket_fd" << endl;
        for (auto node = mloader->slave_addr_to_socket_.begin();
             node != mloader->slave_addr_to_socket_.end(); node++)
          cout << "ip:" << node->first.ip << ", socket fd:" << node->second
               << endl;
        /*
                    /// test whether socket works well
                    ostringstream oss;
                    oss << "hello, i'm master, whose address is "
                        << mloader->master_loader_ip << ":"
                        << to_string(mloader->master_loader_port) << ". \0";

                    int message_length = oss.str().length();
                    DLOG(INFO) << "message length is " << message_length;

                    if (-1 ==
                        write(new_slave_fd,
               reinterpret_cast<char*>(&message_length), 4)) {
                      PLOG(ERROR) << "failed to send message length to slave("
           <<
             ip
               << ":"
                                  << port << ")";
                    } else {
                      DLOG(INFO) << "message length is sent";
                    }
                    if (-1 == write(new_slave_fd, oss.str().c_str(),
               message_length)) {
                      PLOG(ERROR) << "failed to send message to slave(" << ip <<
             ":"
               << port
                                  << ")";
                    } else {
                      DLOG(INFO) << "message buffer is sent";
                    }
         */
        return 1;
      },
      [=](LoadAckAtom, uint64_t txn_id, bool is_commited) {  // NOLINT

        /*
              TODO(ANYONE): there should be a thread checking whether
             transaction overtime periodically and abort these transaction
             and delete from map.
             Consider that: if this function access the item in map just deleted
             by above thread, unexpected thing happens.
         */

        DLOG(INFO) << "received a commit result " << is_commited
                   << " of txn with id:" << txn_id;

        //        cout << "(" << syscall(__NR_gettid) << ")received a commit
        //        result "
        //             << is_commited << "of txn with id:" << txn_id << endl;
        try {
          mloader->commit_info_spin_lock_.acquire();
          CommitInfo& commit_info = mloader->txn_commint_info_.at(txn_id);
          mloader->commit_info_spin_lock_.release();

          if (is_commited) {
            __sync_add_and_fetch(&commit_info.commited_part_num_, 1);
          } else {
            __sync_add_and_fetch(&commit_info.abort_part_num_, 1);
          }
          if (commit_info.IsFinished()) {
            if (0 == commit_info.abort_part_num_) {
              DLOG(INFO) << "going to commit txn with id:" << txn_id << endl;
              TxnClient::CommitIngest(txn_id);
              DLOG(INFO) << "committed txn with id:" << txn_id
                         << " to txn manager";
            } else {
              DLOG(INFO) << "going to abort txn with id:" << txn_id << endl;
              TxnClient::AbortIngest(txn_id);
              DLOG(INFO) << "aborted txn with id:" << txn_id
                         << " to txn manager";
            }

            LOG(INFO) << "finished txn with id:" << txn_id;
            mloader->commit_info_spin_lock_.acquire();
            mloader->txn_commint_info_.erase(txn_id);
            mloader->commit_info_spin_lock_.release();

          // FOR DEBUG
#ifdef MASTER_LOADER_PREF
            if (++debug_finished_txn_count == txn_count_for_debug) {
              cout << "\n" << txn_count_for_debug << " txn used "
                   << GetElapsedTimeInUs(start_time) << " us" << endl;
              cout << "average delay of " << txn_count_for_debug
                   << "txn (from applied txn to finished txn) is:"
                   << txn_average_delay_ * 1.0 / txn_count_for_debug << " us"
                   << endl;
              //              cout << "buffer full times:" << buffer_full_time
              //              << endl;
            } else if (debug_finished_txn_count < txn_count_for_debug) {
              txn_average_delay_ +=
                  GetCurrentUs() - mloader->txn_start_time_.at(txn_id);
            }
#endif
          }
        } catch (const std::out_of_range& e) {
          LOG(ERROR) << "no find " << txn_id << " in map";
          cout << "no find " << txn_id << " in map";
          assert(false);
        }
      },
      [=](RegNodeAtom, NodeAddress addr, NodeID node_id) -> int {  // NOLINT
        LOG(INFO) << "get node register info : (" << addr.ip << ":" << addr.port
                  << ") --> " << node_id;
        NodeTracker::GetInstance()->InsertRegisteredNode(node_id, addr);
        //        return caf::make_message(OkAtom::value);
        return 1;
      },
      [=](BindPartAtom, PartitionID part_id,  // NOLINT
          NodeID node_id) -> int {
        LOG(INFO) << "get part bind info (T" << part_id.projection_id.table_id
                  << "P" << part_id.projection_id.projection_off << "G"
                  << part_id.partition_off << ") --> " << node_id;
        Catalog::getInstance()
            ->getTable(part_id.projection_id.table_id)
            ->getProjectoin(part_id.projection_id.projection_off)
            ->getPartitioner()
            ->bindPartitionToNode(part_id.partition_off, node_id);
        return 1;
      },
      caf::others >> [] { LOG(ERROR) << "nothing matched!!!"; }};
}

RetCode MasterLoader::ConnectWithSlaves() {
  int ret = rSuccess;
  try {
    auto monitor_actor = spawn(MasterLoader::Monitor);
    auto listening_actor = spawn(&MasterLoader::ReceiveSlaveReg, this);
    publish(listening_actor, master_loader_port_, nullptr, true);
    DLOG(INFO) << "published in " << master_loader_ip_ << ":"
               << master_loader_port_;
    cout << "published in " << master_loader_ip_ << ":" << master_loader_port_;
  } catch (exception& e) {
    LOG(ERROR) << "publish master loader actor failed" << e.what();
    return rFailure;
  }
  return ret;
}

RetCode MasterLoader::Ingest(const string& message,
                             function<int()> ack_function) {
  static uint64_t get_request_time = 0;
  static uint64_t get_tuple_time = 0;
  static uint64_t merge_tuple_time = 0;
  static uint64_t time_before_txn = 0;
// assert(false);
#ifdef MASTER_LOADER_PREF
  uint64_t temp_message_count =
      __sync_add_and_fetch(&debug_consumed_message_count, 1);
  if (1 == temp_message_count) {
    gettimeofday(&start_time, NULL);
  }
  if (txn_count_for_debug == temp_message_count) {
    cout << txn_count_for_debug << " txn get request used " << get_request_time
         << " us" << endl;
    cout << txn_count_for_debug << " txn get tuples used " << get_tuple_time
         << " us" << endl;
    cout << txn_count_for_debug << " txn merge tuples used " << merge_tuple_time
         << " us" << endl;
    cout << txn_count_for_debug << " txn before txn used " << time_before_txn
         << " us" << endl;
  }
#endif
  DLOG(INFO) << "consumed message :" << debug_consumed_message_count;

  RetCode ret = rSuccess;
  //  string message = GetMessage();
  //  DLOG(INFO) << "get message:\n" << message;

  /// get message from MQ
  GET_TIME_ML(req_start);
  IngestionRequest req;
  EXEC_AND_DLOG(ret, GetRequestFromMessage(message, &req), "got request!",
                "failed to get request");
  //  ATOMIC_ADD(get_request_time, GetElapsedTimeInUs(req_start));

  /// parse message and get all tuples of all partitions, then
  /// check the validity of all tuple in message
  //  GET_TIME_ML(get_tuple_start);
  TableDescriptor* table =
      Environment::getInstance()->getCatalog()->getTable(req.table_name_);
  assert(table != NULL && "table is not exist!");

  vector<vector<vector<void*>>> tuple_buffers_per_part(
      table->getNumberOfProjection());
  for (int i = 0; i < table->getNumberOfProjection(); ++i)
    tuple_buffers_per_part[i].resize(
        table->getProjectoin(i)->getPartitioner()->getNumberOfPartitions());

#ifdef CHECK_VALIDITY
  vector<Validity> columns_validities;
  EXEC_AND_DLOG(ret, GetPartitionTuples(req, table, tuple_buffers_per_part,
                                        columns_validities),
                "got all tuples of every partition",
                "failed to get all tuples of every partition");
  if (ret != rSuccess && ret != claims::common::rNoMemory) {
    // TODO(YUKAI): error handle, like sending error message to client
    LOG(ERROR) << "the tuple is not valid";
    ack_function();
    return rFailure;
  }
#else
  EXEC_AND_DLOG(ret, GetPartitionTuples(req, table, tuple_buffers_per_part),
                "got all tuples of every partition",
                "failed to get all tuples of every partition");
#endif
  //  ATOMIC_ADD(get_tuple_time, GetElapsedTimeInUs(get_tuple_start));

  /// merge all tuple buffers of partition into one partition buffer
  //  GET_TIME_ML(merge_start);
  vector<vector<PartitionBuffer>> partition_buffers(
      table->getNumberOfProjection());
  EXEC_AND_DLOG(ret, MergePartitionTupleIntoOneBuffer(
                         table, tuple_buffers_per_part, partition_buffers),
                "merged all tuple of same partition into one buffer",
                "failed to merge tuples buffers into one buffer");
  //  ATOMIC_ADD(merge_tuple_time, GetElapsedTimeInUs(merge_start));
  ATOMIC_ADD(time_before_txn, GetElapsedTimeInUs(req_start));

  /// start transaction from here
  claims::txn::Ingest ingest;
  EXEC_AND_LOG(ret, ApplyTransaction(table, partition_buffers, ingest),
               "applied transaction: " << ingest.ts_,
               "failed to apply transaction");

  commit_info_spin_lock_.acquire();
  txn_commint_info_.insert(std::pair<const uint64_t, CommitInfo>(
      ingest.ts_, CommitInfo(ingest.strip_list_.size())));

  txn_start_time_.insert(pair<uint64_t, uint64_t>(ingest.ts_, GetCurrentUs()));
  commit_info_spin_lock_.release();
  DLOG(INFO) << "insert txn " << ingest.ts_ << " into map ";

  /// write data log
  EXEC_AND_DLOG(ret, WriteLog(table, partition_buffers, ingest), "written log",
                "failed to write log");

  /// reply ACK to MQ
  EXEC_AND_DLOG(ret, ack_function(), "replied to MQ", "failed to reply to MQ");

  // cout << ingest.ToString() << endl;
  /*  auto data_size = 0L;
    for (auto& part : partition_buffers) {
      for (auto& buffer : part) data_size += buffer.length_;
    }
    cout << "send_data_size:" << data_size << endl;*/
  /// distribute partition load task
  EXEC_AND_DLOG(ret,
                SendPartitionTupleToSlave(table, partition_buffers, ingest),
                "sent every partition data to send queue",
                "failed to send every partition data to queue");

  for (auto& part_list : partition_buffers)
    for (auto& part : part_list) delete part.buffer_;
  assert(rSuccess == ret);

  return ret;
}

RetCode MasterLoader::GetSocketFdConnectedWithSlave(string ip, int port,
                                                    int* connected_fd) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  //  port = 23667;

  struct sockaddr_in slave_addr;
  slave_addr.sin_family = AF_INET;
  slave_addr.sin_port = htons(port);
  slave_addr.sin_addr.s_addr = inet_addr(ip.c_str());

  if (-1 == connect(fd, (struct sockaddr*)(&slave_addr), sizeof(sockaddr_in))) {
    PLOG(ERROR) << "failed to connect socket(" << ip << ":" << port << ")";
    return rFailure;
  }
#ifdef NON_BLOCK_SOCKET
  int flag = fcntl(fd, F_GETFL);
  if (-1 == flag) PLOG(ERROR) << "failed to get fd flag";
  if (-1 == fcntl(fd, F_SETFL, flag | O_NONBLOCK))
    PLOG(ERROR) << "failed to set fd non-blocking";
#endif
  *connected_fd = fd;
  return rSuccess;
}

// get every tuples and add row id for it
RetCode MasterLoader::GetRequestFromMessage(const string& message,
                                            IngestionRequest* req) {
  // TODO(ANYONE): get the real row_id of table
  static uint64_t row_id = 10000000;
  RetCode ret = rSuccess;
  size_t pos = message.find(',', 0);
  req->table_name_ = message.substr(0, pos);
  pos++;
  size_t next_pos = message.find(',', pos);
  req->col_sep_ = message.substr(pos, next_pos - pos);

  pos = next_pos + 1;
  next_pos = message.find(',', pos);
  req->row_sep_ = message.substr(pos, next_pos - pos);
  /*
    logfile << "table:" << req->table_name_ << ",col sep:" << req->col_sep_
            << ",row sep" << req->row_sep_ << endl;
   */

  pos = next_pos + 1;

  {
    string tuple;
    string data_string = message.substr(pos);
    istringstream iss(data_string);
    while (DataIngestion::GetTupleTerminatedBy(iss, tuple, req->row_sep_)) {
      uint64_t allocated_row_id = __sync_add_and_fetch(&row_id, 1);
      req->tuples_.push_back(to_string(allocated_row_id) + req->col_sep_ +
                             tuple);
    }
  }
  /* {
     int row_seq_length = req->row_sep_.length();
     while (string::npos != (next_pos = message.find(req->row_sep_, pos))) {
       uint64_t allocated_row_id = __sync_add_and_fetch(&row_id, 1);
       req->tuples_.push_back(to_string(allocated_row_id) + req->col_sep_ +
                              message.substr(pos, next_pos - pos));
       pos = next_pos + row_seq_length;
     }
   }*/
  //  req->Show();
  return ret;
}

// map every tuple into associate part
#ifdef CHECK_VALIDITY
RetCode MasterLoader::GetPartitionTuples(
    const IngestionRequest& req, const TableDescriptor* table,
    vector<vector<vector<void*>>>& tuple_buffer_per_part,
    vector<Validity>& columns_validities) {
#else
RetCode MasterLoader::GetPartitionTuples(
    const IngestionRequest& req, const TableDescriptor* table,
    vector<vector<vector<void*>>>& tuple_buffer_per_part) {
#endif

  static uint64_t total_get_tuple_time = 0;
  static uint64_t total_to_value_time = 0;

  RetCode ret = rSuccess;
  Schema* table_schema = table->getSchema();
  MemoryGuard<Schema> table_schema_guard(table_schema);
  vector<void*> correct_tuple_buffer;
  // STLMemoryGuard<vector<void*>> guard(correct_tuple_buffer);  // attention!
  // must set RetCode 'ret' before returning error code!!!!
  ThreeLayerSTLGuardWithRetCode<vector<vector<vector<void*>>>>
      return_tuple_buffer_guard(tuple_buffer_per_part, ret);  // attention!

  // check all tuples to be inserted
  int line = 0;
  int table_tuple_length = table_schema->getTupleMaxSize();
  for (auto tuple_string : req.tuples_) {
    //    DLOG(INFO) << "to be inserted tuple:" << tuple_string;
    void* tuple_buffer = Malloc(table_tuple_length);
    if (tuple_buffer == NULL) return claims::common::rNoMemory;
    MemoryGuardWithRetCode<void> guard(tuple_buffer, ret);
#ifdef CHECK_VALIDITY
    if (rSuccess != (ret = table_schema->CheckAndToValue(
                         tuple_string, tuple_buffer, req.col_sep_,
                         RawDataSource::kSQL, columns_validities))) {
      // handle error which stored in the end
      Validity err = columns_validities.back();
      columns_validities.pop_back();
      string validity_info =
          Validity::GenerateDataValidityInfo(err, table, line, "");
      LOG(ERROR) << validity_info;
    }
    // handle all warnings
    for (auto it : columns_validities) {
      string validity_info =
          Validity::GenerateDataValidityInfo(it, table, line, "");
      LOG(WARNING) << "append warning info:" << validity_info;
    }
    if (rSuccess != ret) {
      // clean work is done by guard
      return ret;
    }
    ++line;
#else
    EXEC_AND_RETURN_ERROR(
        ret, table_schema->ToValue(tuple_string, tuple_buffer, req.col_sep_),
        "tuple is invalid." << tuple_string);
#endif
    correct_tuple_buffer.push_back(tuple_buffer);

    /*    for (auto i = 1; i < table_schema->getncolumns(); i++) {
          logfile << table_schema->getColumnValue(tuple_buffer, i) << "|";
        }
        logfile << endl;*/
  }
  // logfile.flush();
  PERFLOG("all tuples are tovalued");

  // map every tuple in different partition
  for (int i = 0; i < table->getNumberOfProjection(); i++) {
    ProjectionDescriptor* prj = table->getProjectoin(i);
    Schema* prj_schema = prj->getSchema();
    MemoryGuard<Schema> guard(prj_schema);
    vector<Attribute> prj_attrs = prj->getAttributeList();
    vector<unsigned> prj_index;
    for (int j = 0; j < prj_attrs.size(); j++) {
      prj_index.push_back(prj_attrs[j].index);
    }
    SubTuple sub_tuple(table_schema, prj_schema, prj_index);

    const int partition_key_local_index =
        prj->getAttributeIndex(prj->getPartitioner()->getPartitionKey());
    unsigned tuple_max_length = prj_schema->getTupleMaxSize();

    for (auto tuple_buffer : correct_tuple_buffer) {
      // extract the sub tuple according to the projection schema
      void* target = Malloc(tuple_max_length);  // newmalloc
      if (target == NULL) {
        assert(false);
        return (ret = claims::common::rNoMemory);
      }
      sub_tuple.getSubTuple(tuple_buffer, target);

      // determine the partition to write the tuple "target"
      void* partition_key_addr =
          prj_schema->getColumnAddess(partition_key_local_index, target);
      int part = prj_schema->getcolumn(partition_key_local_index)
                     .operate->getPartitionValue(
                         partition_key_addr,
                         prj->getPartitioner()->getPartitionFunction());

      tuple_buffer_per_part[i][part].push_back(target);
    }
  }
  for (auto& tuple : correct_tuple_buffer) delete tuple;
  return ret;
}

RetCode MasterLoader::MergePartitionTupleIntoOneBuffer(
    const TableDescriptor* table,
    vector<vector<vector<void*>>>& tuple_buffer_per_part,
    vector<vector<PartitionBuffer>>& partition_buffers) {
  RetCode ret = rSuccess;
  assert(tuple_buffer_per_part.size() == table->getNumberOfProjection() &&
         "projection number is not match!!");
  for (int i = 0; i < tuple_buffer_per_part.size(); ++i) {
    assert(tuple_buffer_per_part[i].size() ==
               table->getProjectoin(i)
                   ->getPartitioner()
                   ->getNumberOfPartitions() &&
           "partition number is not match");
    Schema* schema = table->getProjectoin(i)->getSchema();
    MemoryGuard<Schema> schema_guard(schema);
    int tuple_len = schema->getTupleMaxSize();
    for (int j = 0; j < tuple_buffer_per_part[i].size(); ++j) {
      int tuple_count = tuple_buffer_per_part[i][j].size();
      /*
       * even if it is empty it has to be pushed into buffer, the index in
       * buffer indicates the index of partition
       */
      //  if (0 == tuple_count) continue;

      int buffer_len = tuple_count * tuple_len;
      DLOG(INFO) << "the tuple length of prj:" << i << ",part:" << j
                 << ",table:" << table->getTableName() << " is:" << tuple_len;
      DLOG(INFO) << "tuple size is:" << tuple_count;

      void* new_buffer = Malloc(buffer_len);
      if (NULL == new_buffer) return ret = claims::common::rNoMemory;

      for (int k = 0; k < tuple_count; ++k) {
        memcpy(new_buffer + k * tuple_len, tuple_buffer_per_part[i][j][k],
               tuple_len);
        // release old memory stored tuple buffer
        DELETE_PTR(tuple_buffer_per_part[i][j][k]);
      }
      // push new partition buffer
      partition_buffers[i].push_back(PartitionBuffer(new_buffer, buffer_len));
      tuple_buffer_per_part[i][j].clear();
    }
    tuple_buffer_per_part[i].clear();
  }
  tuple_buffer_per_part.clear();
  return ret;
}

RetCode MasterLoader::ApplyTransaction(
    const TableDescriptor* table,
    const vector<vector<PartitionBuffer>>& partition_buffers,
    claims::txn::Ingest& ingest) {
  RetCode ret = rSuccess;
  uint64_t table_id = table->get_table_id();

  FixTupleIngestReq req;

  for (int i = 0; i < table->getNumberOfProjection(); ++i) {
    ProjectionDescriptor* prj = table->getProjectoin(i);
    Schema* schema = prj->getSchema();
    MemoryGuard<Schema> schema_guard(schema);
    uint64_t tuple_length = schema->getTupleMaxSize();
    for (int j = 0; j < prj->getPartitioner()->getNumberOfPartitions(); ++j) {
      if (partition_buffers[i][j].length_ == 0) continue;
      req.InsertStrip(GetGlobalPartId(table_id, i, j), tuple_length,
                      partition_buffers[i][j].length_ / tuple_length);
      //      DLOG(INFO) << "the length of partition buffer[" << i << "," << j
      //           << "] is:" << partition_buffers[i][j].length_ << std::endl;
    }
  }

  TxnClient::BeginIngest(req, ingest);
  //  cout << req.ToString() << " " << ingest.ToString() << endl;
  return ret;
}

RetCode MasterLoader::WriteLog(
    const TableDescriptor* table,
    const vector<vector<PartitionBuffer>>& partition_buffers,
    const claims::txn::Ingest& ingest) {
  RetCode ret = rSuccess;
  uint64_t table_id = table->get_table_id();

  for (int prj_id = 0; prj_id < partition_buffers.size(); ++prj_id) {
    for (int part_id = 0; part_id < partition_buffers[prj_id].size();
         ++part_id) {
      if (0 == partition_buffers[prj_id][part_id].length_) continue;
      uint64_t global_part_id = GetGlobalPartId(table_id, prj_id, part_id);

      EXEC_AND_DLOG(
          ret, LogClient::Data(global_part_id,
                               ingest.strip_list_.at(global_part_id).first,
                               ingest.strip_list_.at(global_part_id).second,
                               partition_buffers[prj_id][part_id].buffer_,
                               partition_buffers[prj_id][part_id].length_),
          "written data log for partition:" << global_part_id,
          "failed to write data log for partition:" << global_part_id);
    }
  }
  EXEC_AND_DLOG(ret, LogClient::Refresh(), "flushed data log into disk",
                "failed to flush data log");
  return ret;
}

RetCode MasterLoader::ReplyToMQ(const IngestionRequest& req) {
  // TODO(YUKAI)
  return rSuccess;
}

RetCode MasterLoader::SendPartitionTupleToSlave(
    const TableDescriptor* table,
    const vector<vector<PartitionBuffer>>& partition_buffers,
    const claims::txn::Ingest& ingest) {
  RetCode ret = rSuccess;
  uint64_t table_id = table->get_table_id();
  for (int prj_id = 0; prj_id < partition_buffers.size(); ++prj_id) {
    for (int part_id = 0; part_id < partition_buffers[prj_id].size();
         ++part_id) {
      if (0 == partition_buffers[prj_id][part_id].length_) continue;
      uint64_t global_part_id = GetGlobalPartId(table_id, prj_id, part_id);

      int socket_fd = -1;
      EXEC_AND_DLOG_RETURN(ret, SelectSocket(table, prj_id, part_id, socket_fd),
                           "selected the socket",
                           "failed to select the socket");
      assert(socket_fd > 3);

#ifdef SEND_THREAD
      LoadPacket* packet =
          new LoadPacket(socket_fd, ingest.ts_, global_part_id,
                         ingest.strip_list_.at(global_part_id).first,
                         ingest.strip_list_.at(global_part_id).second,
                         partition_buffers[prj_id][part_id].length_,
                         partition_buffers[prj_id][part_id].buffer_);

      EXEC_AND_DLOG_RETURN(ret, packet->Serialize(),
                           "serialized packet into buffer",
                           "failed to serialize packet");

      int queue_index = socket_fd % send_thread_num_;
      assert(queue_index < send_thread_num_);
      {
        LockGuard<SpineLock> guard(packet_queue_lock_[queue_index]);
        packet_queues_[queue_index].push(packet);
      }
      packet_queue_to_send_count_[queue_index].post();
#else
      LoadPacket packet(socket_fd, ingest.ts_, global_part_id,
                        ingest.strip_list_.at(global_part_id).first,
                        ingest.strip_list_.at(global_part_id).second,
                        partition_buffers[prj_id][part_id].length_,
                        partition_buffers[prj_id][part_id].buffer_);

      EXEC_AND_DLOG_RETURN(ret, packet.Serialize(),
                           "serialized packet into buffer",
                           "failed to serialize packet");
      EXEC_AND_DLOG(
          ret,
          SendPacket(socket_fd, packet.packet_buffer_, packet.packet_length_),
          "sent packet of " << packet.txn_id_, "failed to send packet");
#endif
    }
  }
  return ret;
}

RetCode MasterLoader::SelectSocket(const TableDescriptor* table,
                                   const uint64_t prj_id,
                                   const uint64_t part_id, int& socket_fd) {
  RetCode ret = rSuccess;
  NodeID node_id_in_rmm =
      table->getProjectoin(prj_id)->getPartitioner()->getPartitionLocation(
          part_id);
  DLOG(INFO) << "node id is " << node_id_in_rmm;
  NodeAddress addr;
  EXEC_AND_DLOG_RETURN(
      ret, NodeTracker::GetInstance()->GetNodeAddr(node_id_in_rmm, addr),
      "got node address", "failed to get node address");
  DLOG(INFO) << "node address is " << addr.ip << ":" << addr.port;
  addr.port = "";  // the port is used for OLAP, not for loading
  socket_fd = slave_addr_to_socket_[addr];

  /*  cout << "node id:" << node_id_in_rmm << ",node address:" << addr.ip << ":"
     << addr.port << ",socket fd:" << socket_fd << endl;*/
  return ret;
}

RetCode MasterLoader::SendPacket(const int socket_fd,
                                 const void* const packet_buffer,
                                 const uint64_t packet_length) {
  static int sent_packetcount = 0;
  static uint64_t send_total_time = 0;
  size_t total_write_num = 0;

  /// just lock this socket file descriptor
  LockGuard<Lock> guard(socket_fd_to_lock_[socket_fd]);
  //  GET_TIME_ML(send_start);
  while (total_write_num < packet_length) {
    ssize_t write_num = write(
        socket_fd, static_cast<const char*>(packet_buffer) + total_write_num,
        packet_length - total_write_num);
    if (-1 == write_num) {
      if (EAGAIN == errno) {
        cout << "buffer is full, retry..." << buffer_full_time << endl;
        ATOMIC_ADD(buffer_full_time, 1);
        usleep(500);
        continue;
      }
      std::cerr << "failed to send buffer to slave(" << socket_fd
                << "): " << std::endl;
      PLOG(ERROR) << "failed to send buffer to slave(" << socket_fd << "): ";
      return claims::common::rSentMessageError;
    }
    total_write_num += write_num;
  }
// cout << "send data bytes:" << total_write_num << endl;
#ifdef MASTER_LOADER_PREF
//  if (__sync_add_and_fetch(&sent_packetcount, 1) == txn_count_for_debug * 4) {
//    cout << "send " << sent_packetcount << " packets used " << send_total_time
//         << ", average time is:" << send_total_time / sent_packetcount <<
//         endl;
//  } else {
//    ATOMIC_ADD(send_total_time, GetElapsedTimeInUs(send_start));
//  }
#endif
  return rSuccess;
}

void* MasterLoader::SendPacketWork(void* arg) {
  MasterLoader* loader = static_cast<MasterLoader*>(arg);
  int index = __sync_fetch_and_add(&(loader->thread_index_), 1);
  LOG(INFO) << " I got id :" << index;
  assert(index < loader->send_thread_num_);
  while (1) {
    loader->packet_queue_to_send_count_[index].wait();
    LoadPacket* packet = nullptr;
    {
      LockGuard<SpineLock> guard(loader->packet_queue_lock_[index]);
      packet = loader->packet_queues_[index].front();
      loader->packet_queues_[index].pop();
    }

    RetCode ret = rSuccess;
    EXEC_AND_DLOG(
        ret, loader->SendPacket(packet->socket_fd_, packet->packet_buffer_,
                                packet->packet_length_),
        "sent packet of " << packet->txn_id_, "failed to send packet");
    DELETE_PTR(packet);
  }
}

void* MasterLoader::Work(void* arg) {
  WorkerPara* para = static_cast<WorkerPara*>(arg);
  AMQConsumer consumer(para->brokerURI_, para->destURI_, para->use_topic_,
                       para->client_ack_);
  consumer.run(para->master_loader_);
  while (1) sleep(10);
  return NULL;
}

void* MasterLoader::StartMasterLoader(void* arg) {
  remove("master_loader_log.txt");
  logfile.open("master_loader_log.txt");
  Config::getInstance();
  LOG(INFO) << "start master loader...";

  int ret = rSuccess;
  MasterLoader* master_loader = Environment::getInstance()->get_master_loader();
  EXEC_AND_ONLY_LOG_ERROR(ret, master_loader->ConnectWithSlaves(),
                          "failed to connect all slaves");

  activemq::library::ActiveMQCPP::initializeLibrary();
  // Use either stomp or openwire, the default ports are different for each
  //
  // Examples:
  //    tcp://127.0.0.1:61616                      default to openwire
  //    tcp://127.0.0.1:61616?wireFormat=openwire  same as above
  //    tcp://127.0.0.1:61613?wireFormat=stomp     use stomp instead
  //
  /*  std::string brokerURI =
        "failover:(tcp://"
        "58.198.176.92:61616?wireFormat=openwire&connection.useAsyncSend=true"

        //        "&transport.commandTracingEnabled=true"
        //        "&transport.tcpTracingEnabled=true"
        //        "&wireFormat.tightEncodingEnabled=true"
        ")";*/
  std::string brokerURI = "failover:(tcp://" + Config::amq_url +
                          "?wireFormat=openwire&connection.useAsyncSend=true"

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
      Config::amq_topic +
      "?consumer.prefetchSize = 1 ";  // ?consumer.prefetchSize=1";

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
  bool client_ack = true;

  cout << "\n input a number to continue" << std::endl;
  int temp;
  cin >> temp;
  cout << "Well , start flag is received" << std::endl;

#ifdef SEND_THREAD
  for (int i = 0; i < master_loader->send_thread_num_; ++i) {
    Environment::getInstance()->getThreadPool()->AddTask(
        MasterLoader::SendPacketWork, master_loader);
  }
#endif

  //  AMQConsumer consumer(brokerURI, destURI, use_topics, client_ack);
  //  consumer.run(master_loader);
  WorkerPara para(master_loader, brokerURI, destURI, use_topics, client_ack);
  for (int i = 0; i < Config::master_loader_thread_num - 1; ++i) {
    //    Environment::getInstance()->getThreadPool()->AddTaskInCpu(
    //        MasterLoader::Work, &para, (i + 1) % GetNumberOfCpus());
    Environment::getInstance()->getThreadPool()->AddTask(MasterLoader::Work,
                                                         &para);
  }
  // i am also a worker
  Work(&para);

  while (1) sleep(10);

  //      while (true) EXEC_AND_ONLY_LOG_ERROR(ret, master_loader->Ingest(),
  //                                           "failed to ingest data");

  return NULL;
}

} /* namespace loader */
} /* namespace claims */
