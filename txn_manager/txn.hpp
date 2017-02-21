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
 * /txn/txn_utility.hpp
 *
 *  Created on: 2016年3月28日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */

#ifndef TXN_HPP_
#define TXN_HPP_
#include <vector>
#include <set>
#include <tuple>
#include <iostream>
#include <functional>
#include <algorithm>
#include <memory>
#include <map>
#include <utility>
#include <unordered_map>
#include <time.h>
#include <chrono>
#include <sys/time.h>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "../common/error_define.h"
#include "../utility/Timer.h"
#include "../common/ids.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
namespace claims {
namespace txn {

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;
using std::set;
using std::tuple;
using std::get;
using std::unordered_map;
using std::to_string;
using std::function;
using std::sort;
using std::make_pair;
using std::make_tuple;
using claims::common::rSuccess;

using UInt64 = unsigned long long;
using UInt32 = unsigned int;
using UInt16 = unsigned short;
using UInt8 = char;
using RetCode = int;
using TimerAtom = caf::atom_constant<caf::atom("Timer")>;
using BeginAtom = caf::atom_constant<caf::atom("Begin")>;
using CommitAtom = caf::atom_constant<caf::atom("Commit")>;
using AbortAtom = caf::atom_constant<caf::atom("Abort")>;
using DataAtom = caf::atom_constant<caf::atom("Data")>;

using OkAtom = caf::atom_constant<caf::atom("Ok")>;
using FailAtom = caf::atom_constant<caf::atom("Fail")>;
using IngestAtom = caf::atom_constant<caf::atom("Ingest")>;
using WriteAtom = caf::atom_constant<caf::atom("Write")>;

using DebugAtom = caf::atom_constant<caf::atom("Debug")>;
using QueryAtom = caf::atom_constant<caf::atom("Query")>;
using CommitQueryAtom = caf::atom_constant<caf::atom("CommitQu")>;
using CheckpointAtom = caf::atom_constant<caf::atom("Checkpoint")>;
using GCAtom = caf::atom_constant<caf::atom("GC")>;
using CommitIngestAtom = caf::atom_constant<caf::atom("CommitInge")>;
using AbortIngestAtom = caf::atom_constant<caf::atom("AbortInge")>;
using CheckpointAtom = caf::atom_constant<caf::atom("Checkpoint")>;
using CommitCPAtom = caf::atom_constant<caf::atom("CommitCP")>;
using AbortCPAtom = caf::atom_constant<caf::atom("AbortCP")>;
using QuitAtom = caf::atom_constant<caf::atom("Quit")>;
using LinkAtom = caf::atom_constant<caf::atom("Link")>;
using RefreshAtom = caf::atom_constant<caf::atom("Refresh")>;
using MergeAtom = caf::atom_constant<caf::atom("Merge")>;

static const int kTxnPort = 8089;
static const string kTxnIp = "127.0.0.1";
static const int kConcurrency = 4;
static const int kTxnBufferSize = 1024 * 10000;
static const int kTxnLowLen = 10;
static const int kTxnHighLen = 54;
static const int kGCTime = 5;
static const int kTimeout = 3;
static const int kBlockSize = 64 * 1024;
static const int kTailSize = sizeof(unsigned);
static const int kTxnBinSize = 10;  // 1024;

/**
 * @brief generate global partition id from
 * <table_id,projection_id,partition_id>
 */
inline UInt64 GetGlobalPartId(UInt64 table_id, UInt64 projeciton_id,
                              UInt64 partition_id) {
  return partition_id + 1000 * (projeciton_id + 1000 * table_id);
}

/**
 * @brief generate global partition id from claims build-in PartitionID
 */
inline UInt64 GetGlobalPartId(PartitionID part) {
  auto table_id = part.projection_id.table_id;
  auto proj_id = part.projection_id.projection_off;
  auto part_id = part.partition_off;
  return GetGlobalPartId(table_id, proj_id, part_id);
}

/**
 * @brief generate table id from global partition id.
 */
inline UInt64 GetTableIdFromGlobalPartId(UInt64 global_partition_id) {
  return global_partition_id / (1000 * 1000);
}

/**
 * @brief generate projection id from global partition id.
 */
inline UInt64 GetProjectionIdFromGlobalPartId(UInt64 global_partition_id) {
  return (global_partition_id % (1000 * 1000)) / 1000;
}

/**
 * @brief generate partition id from global partition id.
 */
inline UInt64 GetPartitionIdFromGlobalPartId(UInt64 global_partition_id) {
  return global_partition_id % (1000);
}

/**
 * @brief: position and offset of complete strip
 */
using PStrip = pair<UInt64, UInt64>;

/**
 * @brief: A tuple to describe set of rows
 * @param: partition id [part_],
 *         position of strip [_pos],
 *         offset of strip[_offset]
 */
class Strip {
 public:
  UInt64 part_;
  UInt64 pos_;
  UInt64 offset_;
  Strip() {}
  Strip(UInt64 pId, UInt64 pos, UInt32 offset)
      : part_(pId), pos_(pos), offset_(offset) {}
  UInt64 get_part() const { return part_; }
  UInt64 get_pos() const { return pos_; }
  UInt64 get_offset() const { return offset_; }
  void set_part(UInt64 part) { part_ = part; }
  void set_pos(UInt64 pos) { pos_ = pos; }
  void set_offset(UInt64 offset) { offset_ = offset; }
  string ToString();
  /** input [strip stream] ==by partition==> output [[strip stream]...] */
  static void Map(vector<Strip> &input, map<UInt64, vector<Strip>> &output);

  /** sort strip stream [input] by order of position */
  static void Sort(vector<Strip> &input);
  static void Sort(vector<PStrip> &input);

  /** merge adjacent strip stream [input], input must be sorted beforehand */
  static void Merge(vector<Strip> &input);
  static void Merge(vector<PStrip> &input);

  /** filter strip stream [input], delete item if predicate is not successful */
  static void Filter(vector<Strip> &input, function<bool(Strip &)> predicate);
  static void Filter(vector<PStrip> &input, function<bool(PStrip &)> predicate);
};

inline bool operator==(const Strip &a, const Strip &b) {
  return a.part_ == b.part_ && a.pos_ == b.pos_ && a.offset_ == b.offset_;
}
/**
 * @brief: Describe the information about transaction state
 * @param: transaction commit state [status_],
 *         real begin timestamp of transaction [realtime_],
 *         strip list of transaction writes [strip_list_]
 */
class Txn {
 public:
  static const int kActive = 0;
  static const int kCommit = 1;
  static const int kAbort = 2;
  int status_ = kActive;
  long realtime_;
  unordered_map<UInt64, PStrip> strip_list_;
  Txn() { realtime_ = GetCurrents(); }
  Txn(const unordered_map<UInt64, PStrip> &strip_list) {
    strip_list_ = strip_list;
    // realtime_ = GetCurrents();
  }
  void Commit() { status_ = kCommit; }
  void Abort() { status_ = kAbort; }
  bool IsCommit() { return status_ == kCommit; }
  bool IsAbort() { return status_ == kAbort; }
  bool IsActive() { return status_ == kActive; }
  string ToString();
};

/**
 * @brief: The transaction information that a query needs.
 * @param: [his_cp_list_] is tuple of <partition, historical checkpoint>
 * @param: [rc_cp_list_] is <partition, real-time checkpoint>.
 *          temporary, *need not to be serialized*
 * @param: [part_pstrips] unmerged strip list after historical checkpoint
 */
class Snapshot {
 public:
  unordered_map<UInt64, UInt64> his_cp_list_;
  unordered_map<UInt64, UInt64> rt_cp_list_;
  unordered_map<UInt64, vector<PStrip>> part_pstrips_;
  string ToString() const;
  void setRtCPS(const unordered_map<UInt64, UInt64> &cps) { rt_cp_list_ = cps; }
  void setHisCPS(const unordered_map<UInt64, UInt64> &cps) {
    his_cp_list_ = cps;
  };
  unordered_map<UInt64, UInt64> getHisCPS() const { return his_cp_list_; }
  // add strip into snapshot, ignore strip beforehand real-time checkpoint
  void setPStrips(const unordered_map<UInt64, vector<PStrip>> &part_pstrips) {
    if (rt_cp_list_.size() > 0) {
      // Need to cut off all strip before ahead real-time checkpoint
      for (auto &pstrips : part_pstrips)
        for (auto &pstrip : pstrips.second)
          if (pstrip.first >= rt_cp_list_[pstrips.first])
            part_pstrips_[pstrips.first].push_back(pstrip);
    } else {
      part_pstrips_ = part_pstrips;
    }
  }
  unordered_map<UInt64, vector<PStrip>> getPStrps() const {
    return part_pstrips_;
  }
  // merge another snapshot or strip list
  void Merge(const vector<Strip> &strips);
  void Merge(const Snapshot &snapshot);
};

inline bool operator==(const Snapshot &lhs, const Snapshot &rhs) {
  return lhs.his_cp_list_ == rhs.his_cp_list_ &&
         lhs.part_pstrips_ == rhs.part_pstrips_;
}

/**
 * @brief: object to request to ingest tuple with fix length
 *         use [InsertStrip] to add a request to ingest number of [tuple_count]
 *         tuple with size of [tuple_Size]
 *  @param: [content_] is  fix tuple part -> <tuple_size, tuple_count>
 */
class FixTupleIngestReq {
 public:
  unordered_map<UInt64, PStrip> content_;
  void InsertStrip(UInt64 part, UInt64 tuple_size, UInt64 tuple_count) {
    content_[part] = make_pair(tuple_size, tuple_count);
  }
  unordered_map<UInt64, PStrip> get_content() const { return content_; }
  void set_content(const unordered_map<UInt64, PStrip> &content) {
    content_ = content;
  }
  string ToString();
};
inline bool operator==(const FixTupleIngestReq &a, const FixTupleIngestReq &b) {
  return a.content_ == b.content_;
}

/**
 * @brief: TM return [Ingest] object to execute data ingestion
 * @param: a write-time/transaction id [ts_] for ingestion transaction,
 *         TM allocates [strip_list_] for ingestion transaction
 */
class Ingest {
 public:
  UInt64 ts_;
  unordered_map<UInt64, PStrip> strip_list_;
  Ingest() {}
  Ingest(const unordered_map<UInt64, PStrip> &strip_list, UInt64 ts)
      : strip_list_(strip_list), ts_(ts) {}
  void InsertStrip(UInt64 part, UInt64 pos, UInt64 offset) {
    strip_list_[part] = make_pair(pos, offset);
  }
  void InsertStrip(const Strip &strip) {
    strip_list_[strip.part_] = make_pair(strip.pos_, strip.offset_);
  }
  UInt64 get_id() const { return ts_; }
  unordered_map<UInt64, PStrip> get_strip_list() const { return strip_list_; }
  void set_id(const UInt64 &id) { ts_ = id; }
  void set_strip_list(const unordered_map<UInt64, PStrip> &stripList) {
    strip_list_ = stripList;
  }
  string ToString();
};
inline bool operator==(const Ingest &lhs, const Ingest &rhs) {
  return lhs.ts_ == rhs.ts_;
}

/**
 * @brief: SQL compiler use [QueryReq] to request a snapshot from TM
 * @param: [part_list_] are partitions needed to scan,
 *         [include_abort_] is whether to scan abort strip(
 *         false for client query, true for create checkpoint).
 */
class QueryReq {
 public:
  vector<UInt64> part_list_;
  bool include_abort_ = false;
  QueryReq() {}
  QueryReq(const vector<UInt64> &part_list) : part_list_(part_list) {}
  void InsertPart(UInt64 part) { part_list_.push_back(part); }
  vector<UInt64> get_part_list() const { return part_list_; }
  bool get_include_abort() const { return include_abort_; }
  void set_part_list(const vector<UInt64> &partList) { part_list_ = partList; }
  void set_include_abort(bool include_abort) { include_abort_ = include_abort; }
  string ToString();
};

inline bool operator==(const QueryReq &lhs, const QueryReq &rhs) {
  return lhs.part_list_ == rhs.part_list_;
}

/**
 * @brief: [Query] is information for scan partition
 * @param: [ts_] is read timestamp,
 *         [snapshot_] is strip list need to scan real-time chunk-list,
 *         [his_cp_list_] is historical checkpoint to scan historical chunk-list
 *
 */
class Query {
 public:
  UInt64 ts_;
  unordered_map<UInt64, vector<PStrip>> snapshot_;
  unordered_map<UInt64, UInt64> his_cp_list_;

  /**
   *  rt_cp_list_  and abort_list_ will never be serialized and send,
   */
  unordered_map<UInt64, UInt64> rt_cp_list_;
  unordered_map<UInt64, vector<PStrip>> abort_list_;
  UInt64 scan_count_ = 0;
  Query() {}
  Query(UInt64 ts, const unordered_map<UInt64, UInt64> &his_cp_list,
        const unordered_map<UInt64, UInt64> &rt_cp_list)
      : ts_(ts), his_cp_list_(his_cp_list), rt_cp_list_(rt_cp_list) {}
  UInt64 getTS() const { return ts_; }
  unordered_map<UInt64, vector<PStrip>> getSnapshot() const {
    return snapshot_;
  }
  unordered_map<UInt64, UInt64> getCPList() const { return his_cp_list_; }
  unordered_map<UInt64, vector<PStrip>> getAbortList() const {
    return abort_list_;
  }

  void setTS(UInt64 ts) { ts_ = ts; }
  void setSnapshot(const unordered_map<UInt64, vector<PStrip>> &sp) {
    snapshot_ = sp;
  }
  void setCPList(const unordered_map<UInt64, UInt64> &cplist) {
    his_cp_list_ = cplist;
  }
  void setAbortList(const unordered_map<UInt64, vector<PStrip>> &abort_list) {
    abort_list_ = abort_list;
  }
  string ToString();
  void GenTxnInfo() {
    for (auto &part_strips : snapshot_)
      scan_snapshot_[part_strips.first] = part_strips.second;
    for (auto &part_cp : his_cp_list_)
      scan_cp_list_[part_cp.first] = part_cp.second;
  }
  map<UInt64, vector<PStrip>> scan_snapshot_;
  map<UInt64, UInt64> scan_cp_list_;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &ts_ &scan_snapshot_ &scan_cp_list_;
  }
};
inline bool operator==(const Query &lhs, const Query &rhs) {
  return lhs.snapshot_ == rhs.snapshot_ && lhs.his_cp_list_ == rhs.his_cp_list_;
}
/**
 * *not used yet*
 */
class CheckpointReq {
 public:
  UInt64 ts_;
  UInt64 part_;
  vector<PStrip> strip_list_;
  UInt64 old_his_cp_;
  CheckpointReq() {}
  CheckpointReq(UInt64 ts, UInt64 part) : ts_(ts), part_(part) {}
  string ToString() const;
  UInt64 getTs() const { return ts_; }
  UInt64 getPart() const { return part_; }
  vector<PStrip> getStripList() const { return strip_list_; }
  UInt64 getOldHisCP() const { old_his_cp_; }
  void setTs(UInt64 ts) { ts_ = ts; }
  void setPart(UInt64 part) { part_ = part; }
  vector<PStrip> setStripList(const vector<PStrip> &strip_list) {
    strip_list_ = strip_list;
  }
  void setOldHisCP(UInt64 old_his_cp) { old_his_cp_ = old_his_cp; }
};
inline bool operator==(const CheckpointReq &lhs, const CheckpointReq &rhs) {
  return lhs.ts_ == rhs.ts_ && lhs.part_ == rhs.part_;
}

/**
 * @brief: store versioned checkpoint
 */
class TsCheckpoint {
 public:
  UInt64 GetHisCP(UInt64 ts) {
    UInt64 cp;
    for (auto &ver_cp : vers_his_cp_)
      if (ver_cp.first > ts)
        break;
      else
        cp = ver_cp.second;
    return cp;
  }
  UInt64 GetRtCP(UInt64 ts) {
    UInt64 cp;
    for (auto &ver_cp : vers_rt_cp_)
      if (ver_cp.first > ts)
        break;
      else
        cp = ver_cp.second;
    return cp;
  }
  void SetHisCP(UInt64 ts, UInt64 cp) { vers_his_cp_[ts] = cp; }
  void SetRtCP(UInt64 ts, UInt64 cp) { vers_rt_cp_[ts] = cp; }
  string ToString();

 private:
  map<UInt64, UInt64> vers_his_cp_;
  map<UInt64, UInt64> vers_rt_cp_;
};

/**
 *@brief: store a list of transaction state in [txn_list_]
 *@param: [ct_commit_]  is number of all committed transaction,
 *@param: [ct_abort] is number of all aborted transaction,
 *@param: [ct_] is number of all transaction, ct_ >= ct_commit_ + ct_abort_
 *@param: [status] is whether this bin has product a "mini snapshot" in
 *        [snapshot_] + [abort_list_]
 */
class TxnBin {
 public:
  // get & set pos-th txn in txn
  Txn GetTxn(int pos) const { return txn_list_[pos]; }
  void SetTxn(int pos, const Txn &txn) {
    txn_list_[pos] = txn;
    ct_++;
  }
  void SetTxn(int pos, const unordered_map<UInt64, PStrip> &strip_list) {
    txn_list_[pos] = Txn(strip_list);
    ct_++;
  }
  // set state tag of a transaction to "Commit" or Abort
  void CommitTxn(int pos) {
    txn_list_[pos].Commit();
    ct_commit_++;
  }
  void AbortTxn(int pos) {
    txn_list_[pos].Abort();
    ct_abort_++;
  }
  /** Is this bin are filled with "terminated"(commit or abort) */
  bool IsFull() const { return ct_commit_ + ct_abort_ == kTxnBinSize; }
  bool IsSnapshot() const { return status_ == true; }
  int Count() const { return ct_; }
  int CountCommit() const { return ct_commit_; }
  int CountAbort() const { return ct_abort_; }
  void GenSnapshot();
  void GenSnapshot(const TxnBin &prev);
  void MergeSnapshot(Query &query) const;
  void MergeTxn(Query &query, int len) const;
  string ToString();
  /** get bin id that transaction [ts] resides,
   * [core_num] is number of cores in txn_manager */
  static UInt64 GetTxnBinID(UInt64 ts, UInt64 core_num) {
    return (ts / core_num) / kTxnBinSize;
  }
  /** get position of transaction [ts] in this bin */
  static UInt64 GetTxnBinPos(UInt64 ts, UInt64 core_num) {
    return (ts / core_num) % kTxnBinSize;
  }
  /** get the max timestamp of this bin can reside */
  static UInt64 GetTxnBinMaxTs(UInt64 txnbin_id, UInt64 core_num,
                               UInt64 core_id) {
    return txnbin_id * kTxnBinSize * core_num + kTxnBinSize + core_id;
  }

  Txn txn_list_[kTxnBinSize];

 private:
  bool status_ = false;
  int ct_ = 0;
  int ct_commit_ = 0;
  int ct_abort_ = 0;
  // If bin is full, a snapshot is generated.
  unordered_map<UInt64, vector<PStrip>> snapshot_;
  unordered_map<UInt64, vector<PStrip>> abort_list_;
};

/**
 * setting CAF serialization
 */
inline void CAFSerConfig() {
  caf::announce<FixTupleIngestReq>("FixTupleIngestReq",
                                   make_pair(&FixTupleIngestReq::get_content,
                                             &FixTupleIngestReq::set_content));
  caf::announce<Ingest>(
      "Ingest", make_pair(&Ingest::get_id, &Ingest::set_id),
      make_pair(&Ingest::get_strip_list, &Ingest::set_strip_list));
  caf::announce<QueryReq>(
      "QueryReq", make_pair(&QueryReq::get_part_list, &QueryReq::set_part_list),
      make_pair(&QueryReq::get_include_abort, &QueryReq::set_include_abort));
  caf::announce<Query>("Query", make_pair(&Query::getTS, &Query::setTS),
                       make_pair(&Query::getSnapshot, &Query::setSnapshot),
                       make_pair(&Query::getCPList, &Query::setCPList));
  caf::announce<Snapshot>(
      "Snapshot", make_pair(&Snapshot::getHisCPS, &Snapshot::setHisCPS),
      make_pair(&Snapshot::getPStrps, &Snapshot::setPStrips));
  caf::announce<CheckpointReq>(
      "CheckpointReq", make_pair(&CheckpointReq::getTs, &CheckpointReq::setTs),
      make_pair(&CheckpointReq::getPart, &CheckpointReq::setPart),
      make_pair(&CheckpointReq::getStripList, &CheckpointReq::setStripList),
      make_pair(&CheckpointReq::getOldHisCP, &CheckpointReq::setOldHisCP));
}
}
}
#endif  //  TXN_HPP_
