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
 * /txn/txn_server.cpp
 *
 *  Created on: 2016年4月10日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */
#include <iostream>
#include <fstream>
#include "txn_server.hpp"
#include "caf/all.hpp"
#include "txn_log.hpp"
#include "../utility/resource_guard.h"
#include "../txn_manager/txn_server.hpp"
using caf::aout;
using std::ofstream;
ofstream txn_log;
namespace claims {
namespace txn {
// using claims::common::rSuccess;
// using claims::common::rLinkTmTimeout;
// using claims::common::rLinkTmFail;
// using claims::common::rBeginIngestTxnFail;
// using claims::common::rBeginQueryFail;
// using claims::common::rBeginCheckpointFail;
// using claims::common::rCommitIngestTxnFail;
// using claims::common::rAbortIngestTxnFail;
// using claims::common::rCommitCheckpointFail;

atomic<UInt64> TimeStamp::now_;

int TxnServer::port_ = kTxnPort;
int TxnServer::concurrency_ = kConcurrency;
caf::actor TxnServer::proxy_;
vector<caf::actor> TxnServer::cores_;
bool TxnServer::active_ = false;

unordered_map<UInt64, atomic<UInt64>> TxnServer::pos_list_;
unordered_map<UInt64, TsCheckpoint> TxnServer::cp_list_;
set<UInt64> TxnServer::active_querys_;

caf::behavior TxnCore::make_behavior() {
  this->delayed_send(this, seconds(3 + core_id_), MergeAtom::value);

  return {
      [this](DebugAtom, string flag) { cout << ToString() << endl; },
      [this](IngestAtom, shared_ptr<Ingest> ingest) -> caf::message {
        RetCode ret = rSuccess;
        auto id = TxnBin::GetTxnBinID(ingest->ts_, TxnServer::concurrency_);
        auto pos = TxnBin::GetTxnBinPos(ingest->ts_, TxnServer::concurrency_);
        txnbin_list_[id].SetTxn(pos, ingest->strip_list_);
        return caf::make_message(ret, *ingest);
      },
      [this](CommitIngestAtom, const UInt64 ts) -> caf::message {
        RetCode ret = rSuccess;
        auto id = TxnBin::GetTxnBinID(ts, TxnServer::concurrency_);
        auto pos = TxnBin::GetTxnBinPos(ts, TxnServer::concurrency_);
        txnbin_list_[id].CommitTxn(pos);
        return caf::make_message(rSuccess);
      },
      [this](AbortIngestAtom, const UInt64 ts) -> caf::message {
        RetCode ret = rSuccess;
        auto id = TxnBin::GetTxnBinID(ts, TxnServer::concurrency_);
        auto pos = TxnBin::GetTxnBinPos(ts, TxnServer::concurrency_);
        txnbin_list_[id].AbortTxn(pos);
        return caf::make_message(rSuccess);
      },
      [this](QueryAtom, shared_ptr<Query> query,
             bool include_abort) -> caf::message {
        int last_core = query->ts_ / TxnServer::concurrency_;
        int bin_num = 0;
        if (query->ts_ >= TxnServer::concurrency_)
          bin_num = (query->ts_) / (TxnServer::concurrency_ * kTxnBinSize);
        int remain = 0;
        if (query->ts_ > TxnServer::concurrency_) {
          auto all_remain =
              (query->ts_) % (TxnServer::concurrency_ * kTxnBinSize);
          remain = (all_remain + TxnServer::concurrency_ - 1 - core_id_) /
                   TxnServer::concurrency_;
        }
        if (core_id_ > last_core && remain > 0) remain--;
        if (!include_abort) {
          cout << "qy_ts:" << query->ts_ << ",core:" << core_id_
               << ",bin num:" << bin_num << ",re:" << remain << endl;
        }
        if (remain > 0) {
          txnbin_list_[bin_num].MergeTxn(*query, remain);
          auto count = txnbin_list_[bin_num].Count();
          if (count < remain)
            cout << "!!!!!error:" << query->ts_ << "@" << core_id_ << ","
                 << bin_num << "?count:" << count << ",remain:" << remain
                 << endl;
        }

        for (int id = bin_num - 1; id >= 0; id--) {
          if (txnbin_list_[id].IsSnapshot()) {
            txnbin_list_[id].MergeSnapshot(*query);
            break;
          } else {
            txnbin_list_[id].MergeTxn(*query, kTxnBinSize);
            auto count = txnbin_list_[id].Count();
            if (count < kTxnBinSize)
              cout << "error:" << query->ts_ << "@" << core_id_ << ","
                   << bin_num << "?count:" << count << endl;
          }
        }
        auto next_core_id = (core_id_ + 1) % TxnServer::concurrency_;
        if (next_core_id != TxnServer::GetCoreID(query->ts_)) {
          // scan next core
          this->forward_to(TxnServer::cores_[next_core_id]);
        } else if (include_abort) {
          for (auto& part_cp : query->rt_cp_list_) {
            auto part = part_cp.first;
            auto checkpoint = part_cp.second;
            // mix snapshot_list into abort_list
            query->abort_list_[part].insert(query->abort_list_[part].end(),
                                            query->snapshot_[part].begin(),
                                            query->snapshot_[part].end());
            // clear abort list
            Strip::Sort(query->abort_list_[part]);
            Strip::Merge(query->abort_list_[part]);
            Strip::Filter(query->abort_list_[part],
                          [checkpoint](PStrip& pstrip) -> bool {
              if (pstrip.first + pstrip.second <= checkpoint) {
                return false;
              } else {
                if (pstrip.first < checkpoint &&
                    pstrip.first + pstrip.second > checkpoint)
                  pstrip = make_pair(checkpoint,
                                     pstrip.first + pstrip.second - checkpoint);
                return true;
              }
            });
            // remove not continuous commit/abort strip
            if (query->abort_list_[part].size() > 0) {
              auto continuous_pos = query->abort_list_[part][0].first +
                                    query->abort_list_[part][0].second;
              Strip::Filter(query->snapshot_[part],
                            [continuous_pos](PStrip& pstrip) -> bool {
                if (pstrip.first + pstrip.second <= continuous_pos)
                  return true;
                else
                  return false;
              });
            }
          }
        }
        return caf::make_message(*query);
      },
      [this](MergeAtom) {
        while (txnbin_list_[txnbin_cur_].IsFull()) {
          if (txnbin_cur_ == 0) {
            txnbin_list_[txnbin_cur_].GenSnapshot();
          } else {
            txnbin_list_[txnbin_cur_].GenSnapshot(
                txnbin_list_[txnbin_cur_ - 1]);
          }
          txnbin_cur_++;
        }
        this->delayed_send(this, seconds(3 + core_id_), MergeAtom::value);
      },
      [this](GCAtom, UInt64 min_ts_remain) {

        map<UInt64, TxnBin> new_txnbin_list;
        auto ct = 0;
        for (auto it = txnbin_list_.rbegin(); it != txnbin_list_.rend(); it++) {
          auto id = it->first;
          auto max_ts =
              TxnBin::GetTxnBinMaxTs(id, TxnServer::concurrency_, core_id_);
          if (txnbin_list_[id].IsSnapshot()) {
            if (ct == 0) {
              new_txnbin_list[id] = txnbin_list_[id];
              ct++;
            } else {
              break;
            }
          } else {
            new_txnbin_list[id] = txnbin_list_[id];
          }
        }
        if (new_txnbin_list.size() < txnbin_list_.size())
          txnbin_list_ = new_txnbin_list;

      },
      [](GCAtom) {
        /*        auto size_old = size_;
                auto pos = 0;
                for (auto i = 0; i < size_; i++)
                  if (!TxnServer::IsStripListGarbage(strip_list_[i])) {
                    txn_index_[txn_index_[i]] = pos;
                    commit_[pos] = commit_[i];
                    abort_[pos] = abort_[i];
                    strip_list_[pos] = strip_list_[i];
                    ++pos;
                  }
                size_ = pos;
                cout << "core:" << core_id_ << ",gc:" << size_old << "=>" << pos
                     << endl;*/
        // this->delayed_send(this, seconds(kGCTime), GCAtom::value);
      },
      caf::others >> [&]() {
                       cout << "core:" << core_id_ << " unkown message "
                            << to_string(current_message()) << endl;
                     }};
}

string TxnCore::ToString() {
  string str = "core_id:" + to_string(core_id_) + "\n";
  for (auto& txnbin : txnbin_list_) {
    str += "txnbin_id:" + to_string(txnbin.first) + "\n";
    str += txnbin.second.ToString();
  }
  return str;
}

caf::behavior TxnServer::make_behavior() {
  if (!txn_log.is_open()) txn_log.open("txn_log.txt");
  try {
    caf::io::publish(proxy_, port_, nullptr, true);
    cout << "txn server bind to port:" << port_ << " success" << endl;
  } catch (...) {
    cout << "txn server bind to port:" << port_ << " fail" << endl;
  }
  // this gc for txn_manager is not complete implemented
  // this->delayed_send(this, seconds(3), GCAtom::value);
  return {[this](DebugAtom, string flag) -> caf::message {
            cout << "debug begin" << endl;
            for (auto& core : cores_)
              caf::anon_send(core, DebugAtom::value, flag);
            return caf::make_message(rSuccess);
          },
          [this](IngestAtom, const FixTupleIngestReq& request) {
            auto ts = TimeStamp::GenAdd();
            // temporary object to store snapshot, etc. between cores
            auto ingest = make_shared<Ingest>(request.content_, ts);
            for (auto& part : ingest->strip_list_)
              // atomic malloc a strip for each requested partition
              ingest->InsertStrip(AtomicMalloc(part.first, part.second.first,
                                               part.second.second));
            current_message() = caf::make_message(IngestAtom::value, ingest);
            // forward request to each "core"
            forward_to(cores_[GetCoreID(ts)]);
          },
          [this](CommitIngestAtom,
                 const UInt64 ts) { forward_to(cores_[GetCoreID(ts)]); },
          [this](AbortIngestAtom,
                 const UInt64 ts) { forward_to(cores_[GetCoreID(ts)]); },
          [this](QueryAtom, const QueryReq& request) {
            auto ts = TimeStamp::Gen();
            active_querys_.insert(ts);
            auto query =
                make_shared<Query>(ts, GetHisCPList(ts, request.part_list_),
                                   GetRtCPList(ts, request.part_list_));
            current_message() = caf::make_message(QueryAtom::value, query,
                                                  request.include_abort_);
            // forward request to each core to get sub snapshot
            forward_to(cores_[GetCoreID(ts)]);
          },
          [this](CommitQueryAtom, UInt64 ts) -> caf::message {
            // query execution end
            active_querys_.erase(ts);
            return caf::make_message(rSuccess);
          },
          [this](CommitCPAtom, UInt64 ts, UInt64 part, UInt64 his_cp,
                 UInt64 rt_cp) -> caf::message {
            // commit a checkpoint on partition, set new one <his_cp, rt_cp>
            cp_list_[part].SetHisCP(ts, his_cp);
            cp_list_[part].SetRtCP(ts, rt_cp);
            // cout << "commit " << part << ":" << his_cp << "," << rt_cp <<
            // endl;
            return caf::make_message(rSuccess);
          },
          [this](GCAtom) {
            UInt64 ts;
            if (active_querys_.size() > 0)
              ts = *active_querys_.begin();
            else
              ts = TimeStamp::Gen();
            for (auto& core : TxnServer::cores_)
              caf::anon_send(core, GCAtom::value, ts);
            this->delayed_send(this, seconds(3), GCAtom::value);
          },
          caf::others >> [this]() {
                           cout << "server unkown message:"
                                << to_string(current_message()) << endl;
                         }};
}

RetCode TxnServer::Init(int concurrency, int port) {
  active_ = true;
  concurrency_ = concurrency;
  port_ = port;
  proxy_ = caf::spawn<TxnServer>();
  // start all cores
  for (auto i = 0; i < concurrency_; i++)
    cores_.push_back(caf::spawn<TxnCore, caf::detached>(i));
  CAFSerConfig();
  return 0;
}

Strip TxnServer::AtomicMalloc(UInt64 part, UInt64 TupleSize,
                              UInt64 TupleCount) {
  Strip strip;
  strip.part_ = part;
  if (TupleSize * TupleCount == 0) return strip;
  do {
    strip.pos_ = pos_list_[part].load();
    strip.offset_ = 0;
    UInt64 block_pos = strip.pos_ % kBlockSize;
    UInt64 remain_count = TupleCount;
    int count = 0;
    while (remain_count > 0) {
      // Calculate max tuple count that a block resides
      UInt64 use_count = (kBlockSize - block_pos - kTailSize) / TupleSize;
      if (use_count > remain_count) use_count = remain_count;

      // update remain count to be allocated
      remain_count -= use_count;
      strip.offset_ += use_count * TupleSize;
      block_pos += use_count * TupleSize;

      // allocate unavailable space of block tail
      if (kBlockSize - block_pos - kTailSize < TupleSize) {
        strip.offset_ += kBlockSize - block_pos;
        block_pos = 0;
      }
    }
  } while (!pos_list_[part].compare_exchange_weak(strip.pos_,
                                                  strip.pos_ + strip.offset_));
  return strip;
}

unordered_map<UInt64, UInt64> TxnServer::GetHisCPList(
    UInt64 ts, const vector<UInt64>& parts) {
  unordered_map<UInt64, UInt64> cps;
  for (auto& part : parts) cps[part] = cp_list_[part].GetHisCP(ts);
  return cps;
}

unordered_map<UInt64, UInt64> TxnServer::GetRtCPList(
    UInt64 ts, const vector<UInt64>& parts) {
  unordered_map<UInt64, UInt64> cps;
  for (auto& part : parts) cps[part] = cp_list_[part].GetRtCP(ts);
  return cps;
}

RetCode TxnServer::LoadCPList(UInt64 ts,
                              const unordered_map<UInt64, UInt64>& his_cp_list,
                              const unordered_map<UInt64, UInt64>& rt_cp_list) {
  for (auto& cp : his_cp_list) cp_list_[cp.first].SetHisCP(ts, cp.second);
  for (auto& cp : rt_cp_list) cp_list_[cp.first].SetRtCP(ts, cp.second);
  return rSuccess;
}

RetCode TxnServer::LoadPos(const unordered_map<UInt64, UInt64>& pos_list) {
  for (auto& pos : pos_list) pos_list_[pos.first].store(pos.second);
  return rSuccess;
}
}
}
