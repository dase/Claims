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
 * /txn/txn_client.hpp
 *
 *  Created on: 2016年4月10日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */

#ifndef TXN_CLIENT_HPP_
#define TXN_CLIENT_HPP_
#include <vector>
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
#include "unistd.h"
#include "stdlib.h"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "txn.hpp"
#include "txn_server.hpp"

#include <chrono>

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;
using std::to_string;
using std::function;
using std::sort;
using std::atomic;
using std::chrono::seconds;
using std::chrono::milliseconds;

namespace claims {
namespace txn {
/**
 * @brief: Client  APIs to transactional operate data ingestion
 * @param: [ip_],[port_] are network address of TxnServer
 * @param: [proxy_] is remote actor that handles request to TxnServer
 */
class TxnClient {
 public:
  static string ip_;
  static int port_;
  static caf::actor proxy_;

  /** Initialize [TxnClient] that [TxnServer] work at <ip, port> */
  static RetCode Init(string ip = kTxnIp, int port = kTxnPort);

  /** Just for debug, print some information */
  static RetCode Debug(string flag);

  /** Use [request] as parameter to request a ingestion transaction from
   * TxnServer.
   * The [ingest] is assigned to transaction information after function called.*/
  static RetCode BeginIngest(const FixTupleIngestReq& request, Ingest& ingest);

  /** Commit ingestion transaction [ts].
   * [ts] is write timestamp of ingestion transaction,
   * set its visibility to true. */
  static RetCode CommitIngest(UInt64 ts);

  /** Abort ingestion transaction [ts].
   * [id] is write timestamp of ingestion transaction. */
  static RetCode AbortIngest(UInt64 ts);

  /** Use [request] as parameter to request a query transaction from TxnServer.
   * [query] assigned to transaction information after function called */
  static RetCode BeginQuery(const QueryReq& request, Query& query);

  /** Announce query transaction [ts] is end*/
  static RetCode CommitQuery(UInt64 ts);

  /** Commit checkpoint [ts] on [part],
   * set new historical checkpoint [his_cp], and real-time checkpoint [rt_cp] */
  static RetCode CommitCheckpoint(UInt64 ts, UInt64 part, UInt64 his_cp,
                                  UInt64 rt_cp);
};
}
}

#endif  //  TXN_CLIENT_HPP_
