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
 * /log/log.cpp
 *
 *  Created on: 2016年2月24日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */
#include "txn_log.hpp"

namespace claims {
namespace txn {

string LogServer::file_path_ = ".";
FILE* LogServer::file_handler_ = nullptr;
UInt64 LogServer::file_size_ = 0;
UInt64 LogServer::file_capacity_ = kMaxLogSize;
char* LogServer::buffer_ = nullptr;
UInt64 LogServer::buffer_size_ = 0;
UInt64 LogServer::buffer_capacity_ = kMaxLogSize * 10;
caf::actor LogServer::proxy_;
bool LogServer::active_ = false;
RetCode LogServer::Init(const string path) {
  cout << "log server init" << endl;
  proxy_ = caf::spawn<LogServer, caf::detached>();
  file_path_ = path;
  buffer_ = (char*)malloc(buffer_capacity_);
  if (buffer_ == nullptr) return -1;
  active_ = true;
  return 0;
}

caf::behavior LogServer::make_behavior() {
  return {
      [=](IngestAtom, shared_ptr<Ingest> ingest) -> caf::message {
        Append(BeginLog(ingest->ts_));
        for (auto& strip : ingest->strip_list_)
          Append(WriteLog(ingest->ts_, strip.first, strip.second.first,
                          strip.second.second));
        // cout << "begin" << endl;
        return caf::make_message(0, *ingest);
      },
      [=](CommitIngestAtom, const UInt64 id) -> caf::message {
        Append(CommitLog(id));
        // cout << "commit" << endl;
        Refresh();
        return caf::make_message(0);
      },
      [=](AbortIngestAtom, UInt64 id) -> caf::message {
        Append(AbortLog(id));
        // cout << "abort" << endl;
        Refresh();
        return caf::make_message(0);
      },
      [=](CheckpointAtom, UInt64 part, UInt64 logic_cp, UInt64 phy_cp)
          -> RetCode { return Append(CheckpointLog(part, logic_cp, phy_cp)); },
      [=](DataAtom, UInt64 part, UInt64 pos, UInt64 offset, void* buffer,
          UInt64 size) -> caf::message {
        Append(DataLogPrefix(part, pos, offset, size));
        Append(buffer, size);
        return caf::make_message(0);
      },
      [=](RefreshAtom) -> caf::message {

        Refresh();
        return caf::make_message(0);
      },
      caf::others >> [=]() { cout << "unknown log message" << endl; }};
}

RetCode LogServer::Append(const string& log) {
  if (buffer_size_ + log.length() >= buffer_capacity_) {
    cout << "append fail" << endl;
    return -1;
  }
  memcpy(buffer_ + buffer_size_, log.c_str(), log.length());
  buffer_size_ += log.length();
  file_size_ += log.length();
  return 0;
}

RetCode LogServer::Append(void* data, UInt64 size) {
  if (buffer_size_ + size >= buffer_capacity_) return -1;

  memcpy(buffer_ + buffer_size_, data, size);
  buffer_size_ += size;
  buffer_[buffer_size_++] = '\n';
  file_size_ += size + 1;

  return 0;
}

RetCode LogServer::Refresh() {
  // cout << "refresh" << endl;
  if (file_handler_ == nullptr) {
    struct timeval ts;
    gettimeofday(&ts, NULL);
    string file = file_path_ + "/" + kTxnLogFileName + to_string(ts.tv_sec);
    // cout << file << endl;
    file_handler_ = fopen(file.c_str(), "a");
    if (file_handler_ == nullptr) {
      // cout <<"open file fail"<<endl;
      return -1;
    }
  }

  if (buffer_size_ == 0) return 0;
  // cout << buffer_size << endl;
  fwrite(buffer_, sizeof(char), buffer_size_, file_handler_);
  fflush(file_handler_);
  buffer_size_ = 0;

  /* 日志文件已满 */
  if (file_size_ >= file_capacity_) {
    if (file_handler_ == nullptr) return -1;
    fclose(file_handler_);
    file_handler_ = nullptr;
    file_size_ = 0;
  }
  return 0;
}

RetCode LogClient::Begin(UInt64 id) {
  //  RetCode ret = 0;
  //  caf::scoped_actor self;
  //  cout<<"going to send begin atom to log server
  //  :"<<LogServer::proxy_.id()<<endl;
  //  self->sync_send( log_s,BeginAtom::value, id).
  //      await( [&](RetCode ret_code) { cout<<"log:Begin,
  //      ret"<<ret_code<<endl;ret = ret_code;});
  //  return ret;
}
RetCode LogClient::Write(UInt64 id, UInt64 part, UInt64 pos, UInt64 offset) {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, WriteAtom::value, id, part, pos, offset)
        .await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}
RetCode LogClient::Commit(UInt64 id) {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, CommitAtom::value, id)
        .await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}
RetCode LogClient::Abort(UInt64 id) {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, AbortAtom::value, id)
        .await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}
RetCode LogClient::Data(UInt64 part, UInt64 pos, UInt64 offset, void* buffer,
                        UInt64 size) {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, DataAtom::value, part, pos, offset,
                    buffer,
                    size).await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}
RetCode LogClient::Checkpoint(UInt64 part, UInt64 logic_cp, UInt64 phy_cp) {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, CheckpointAtom::value, part, logic_cp,
                    phy_cp).await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}

RetCode LogClient::Refresh() {
  RetCode ret = 0;
  caf::scoped_actor self;
  if (LogServer::active_)
    self->sync_send(LogServer::proxy_, RefreshAtom::value)
        .await([&](RetCode ret_code) { ret = ret_code; });
  return ret;
}
}
}
