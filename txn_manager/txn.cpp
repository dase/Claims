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
 * /txn/txn_utility.cpp
 *
 *  Created on: 2016年3月28日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */
#include "txn.hpp"
#include "txn_server.hpp"

namespace claims {
namespace txn {

using claims::txn::Strip;

string Txn::ToString() {
  string str = "";
  for (auto &strip : strip_list_)
    str += "<" + to_string(strip.first) + "," + to_string(strip.second.first) +
           "," + to_string(strip.second.second) + ">,";
  str += "\n";
  return str;
}
string Snapshot::ToString() const {
  string str = "*****snapshot*****\n";
  for (auto part_cp : part_pstrips_) {
    str += "part:" + to_string(part_cp.first) + " ";
    for (auto pstrip : part_cp.second)
      str +=
          "<" + to_string(pstrip.first) + "," + to_string(pstrip.second) + ">";
  }
  return str + "\n******************\n";
}

void Strip::Map(vector<Strip> &input, map<UInt64, vector<Strip>> &output) {
  output.clear();
  for (auto &strip : input) {
    if (output.find(strip.part_) != output.end())
      output[strip.part_].push_back(strip);
    else
      output[strip.part_] = vector<Strip>();
  }
}

void Strip::Sort(vector<Strip> &input) {
  sort(input.begin(), input.end(),
       [](const Strip &a, const Strip &b) { return a.pos_ < b.pos_; });
}

void Strip::Sort(vector<PStrip> &input) {
  sort(input.begin(), input.end(),
       [](const PStrip &a, const PStrip &b) { return a.first < b.first; });
}

void Strip::Merge(vector<Strip> &input) {
  vector<Strip> buffer(input);
  input.clear();
  if (buffer.size() == 0) return;
  auto pid = buffer[0].part_;
  auto begin = buffer[0].pos_;
  auto end = buffer[0].pos_ + buffer[0].offset_;
  for (auto i = 1; i < buffer.size(); i++) {
    if (end == buffer[i].pos_) {
      end = buffer[i].pos_ + buffer[i].offset_;
    } else {
      input.emplace_back(pid, begin, end - begin);
      begin = buffer[i].pos_;
      end = begin + buffer[i].offset_;
    }
  }
  input.emplace_back(pid, begin, end - begin);
}

void Strip::Merge(vector<PStrip> &input) {
  if (input.size() == 0) return;
  vector<PStrip> buffer;
  auto begin = input[0].first;
  auto end = input[0].first + input[0].second;
  for (auto i = 1; i < input.size(); i++) {
    if (end == input[i].first) {
      end = input[i].first + input[i].second;
    } else {
      buffer.emplace_back(begin, end - begin);
      begin = input[i].first;
      end = input[i].first + input[i].second;
    }
  }
  buffer.emplace_back(begin, end - begin);
  input = buffer;
}

void Strip::Filter(vector<Strip> &input, function<bool(Strip &)> predicate) {
  vector<Strip> buffer(input);
  input.clear();
  for (auto &strip : buffer)
    if (predicate(strip)) input.push_back(strip);
}

void Strip::Filter(vector<PStrip> &input, function<bool(PStrip &)> predicate) {
  vector<PStrip> buffer(input);
  input.clear();
  for (auto &pstrip : buffer)
    if (predicate(pstrip)) input.push_back(pstrip);
}

string Strip::ToString() {
  string str = "*******Strip******\n";
  str += "part:" + to_string(part_) + ",pos:" + to_string(pos_) + ",Offset:" +
         to_string(offset_) + "\n";
  return str;
}

string FixTupleIngestReq::ToString() {
  string str = "*******FixTupleIngestReq********\n";
  for (auto &item : content_)
    str += "part:" + to_string(item.first) + ",tuple_size:" +
           to_string(item.second.first) + ",tuple_count:" +
           to_string(item.second.second) + "\n";
  return str;
}
string Ingest::ToString() {
  UInt64 core_id = TxnServer::GetCoreID(ts_);
  UInt64 txnbin_id = TxnBin::GetTxnBinID(ts_, TxnServer::concurrency_);
  UInt64 txnbin_pos = TxnBin::GetTxnBinPos(ts_, TxnServer::concurrency_);
  string str = "*******Ingest*********\n";
  str += "TS:" + to_string(ts_) + ",core:" + to_string(core_id) +
         ",txnbin_id:" + to_string(txnbin_id) + ",txnbin_pos:" +
         to_string(txnbin_pos) + "\n";
  for (auto &item : strip_list_)
    str += "part:" + to_string(item.first) + ",pos:" +
           to_string(item.second.first) + ",offset:" +
           to_string(item.second.second) + "\n";
  return str;
}
string QueryReq::ToString() {
  string str = "*******QueryReq********\n";
  for (auto &part : part_list_) str += "part:" + to_string(part) + "\n";
  return str;
}

string Query::ToString() {
  string str = "******Query:" + to_string(ts_) + "*******\n";
  for (auto &part : snapshot_) {
    str += "part:" + to_string(part.first) + " ";
    for (auto &strip : part.second)
      str +=
          "<" + to_string(strip.first) + "," + to_string(strip.second) + ">,";
    str += "\n";
  }
  return str;
}

string TsCheckpoint::ToString() {
  string str = "*******checkpoint*******\n";
  str += "Historical:";
  for (auto &cp : vers_his_cp_)
    str += "<" + to_string(cp.first) + "," + to_string(cp.second) + ">";
  str += "\nReal-time:";
  for (auto &cp : vers_rt_cp_)
    str += "<" + to_string(cp.first) + "," + to_string(cp.second) + ">";
  str += "\n";
  return str;
}

string CheckpointReq::ToString() const {
  string str = "*****CheckpointReq******\n";
  str += "ts:" + to_string(ts_) + ",part:" + to_string(part_) + ",old_his_cp:" +
         to_string(old_his_cp_) + "\n";
  str += "strip_list:";
  for (auto &strip : strip_list_)
    str += "<" + to_string(strip.first) + "," + to_string(strip.second) + ">,";
  return str + "\n";
}

void Snapshot::Merge(const vector<Strip> &strips) {
  for (auto &strip : strips)
    part_pstrips_[strip.part_].push_back(PStrip(strip.pos_, strip.offset_));
  for (auto &part_strips : part_pstrips_) {
    Strip::Sort(part_strips.second);
    Strip::Merge(part_strips.second);
  }
}

void Snapshot::Merge(const Snapshot &snapshot) {
  for (auto &part_pstrip : snapshot.part_pstrips_) {
    part_pstrips_[part_pstrip.first].insert(
        part_pstrips_[part_pstrip.first].end(), part_pstrip.second.begin(),
        part_pstrip.second.end());
  }
  for (auto &part_strips : part_pstrips_) {
    Strip::Sort(part_strips.second);
    Strip::Merge(part_strips.second);
  }
}
void TxnBin::MergeSnapshot(Query &query) const {
  for (auto &part_cp : query.rt_cp_list_) {
    auto part = part_cp.first;
    auto checkpoint = part_cp.second;
    query.snapshot_[part].insert(query.snapshot_[part].end(),
                                 snapshot_[part].begin(),
                                 snapshot_[part].end());
    Strip::Sort(query.snapshot_[part]);
    Strip::Merge(query.snapshot_[part]);
    Strip::Filter(query.snapshot_[part], [checkpoint](PStrip &pstrip) -> bool {
      if (pstrip.first + pstrip.second <= checkpoint) {
        return false;
      } else {
        if (pstrip.first < checkpoint &&
            pstrip.first + pstrip.second > checkpoint)
          pstrip =
              make_pair(checkpoint, pstrip.first + pstrip.second - checkpoint);
        return true;
      }
    });
    query.abort_list_[part].insert(query.abort_list_[part].end(),
                                   abort_list_[part].begin(),
                                   abort_list_[part].end());
  }
}

void TxnBin::MergeTxn(Query &query, int len) const {
  for (auto i = 0; i < len; i++)
    if (txn_list_[i].IsCommit()) {
      query.scan_count_++;
      for (auto &strip : txn_list_[i].strip_list_)
        query.snapshot_[strip.first].push_back(strip.second);
    } else if (txn_list_[i].IsAbort()) {
      assert(false);
      for (auto &strip : txn_list_[i].strip_list_)
        query.abort_list_[strip.first].push_back(strip.second);
    }

  for (auto &part_cp : query.rt_cp_list_) {
    auto part = part_cp.first;
    auto checkpoint = part_cp.second;
    Strip::Sort(query.snapshot_[part]);
    Strip::Merge(query.snapshot_[part]);
    Strip::Filter(query.snapshot_[part], [checkpoint](PStrip &pstrip) -> bool {
      if (pstrip.first + pstrip.second <= checkpoint) {
        return false;
      } else {
        if (pstrip.first < checkpoint &&
            pstrip.first + pstrip.second > checkpoint)
          pstrip =
              make_pair(checkpoint, pstrip.first + pstrip.second - checkpoint);
        return true;
      }
    });
  }
}

string TxnBin::ToString() {
  string str = "";
  for (auto i = 0; i < kTxnBinSize; i++)
    if (txn_list_[i].IsCommit() || txn_list_[i].IsAbort())
      str += "txnbin_pos:" + to_string(i) + txn_list_[i].ToString();
  return str;
}

void TxnBin::GenSnapshot(const TxnBin &prev) {
  status_ = true;
  snapshot_ = prev.snapshot_;
  for (auto &txn : txn_list_)
    if (txn.IsCommit()) {
      for (auto &strip : txn.strip_list_)
        snapshot_[strip.first].push_back(strip.second);
    } else if (txn.IsAbort()) {
      for (auto &strip : txn.strip_list_)
        abort_list_[strip.first].push_back(strip.second);
    }
  for (auto &part : snapshot_) {
    Strip::Sort(part.second);
    Strip::Merge(part.second);
  }
  for (auto &part : abort_list_) {
    Strip::Sort(part.second);
    Strip::Merge(part.second);
  }
}

void TxnBin::GenSnapshot() {
  status_ = true;
  for (auto &txn : txn_list_)
    if (txn.IsCommit()) {
      for (auto &strip : txn.strip_list_)
        snapshot_[strip.first].push_back(strip.second);
    } else if (txn.IsAbort()) {
      for (auto &strip : txn.strip_list_)
        abort_list_[strip.first].push_back(strip.second);
    }
  for (auto &part : snapshot_) {
    Strip::Sort(part.second);
    Strip::Merge(part.second);
  }
  for (auto &part : abort_list_) {
    Strip::Sort(part.second);
    Strip::Merge(part.second);
  }
}
}  // namespace txn
}  // namespace claims
