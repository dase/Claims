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
 * /txn/txn_client.cpp
 *
 *  Created on: 2016年4月10日
 *      Author: imdb
 *		   Email:
 *
 * Description:
 *
 */

#include "txn_client.hpp"
#include "txn_log.hpp"
namespace claims {
namespace txn {

// using claims::txn::TxnClient;
// using claims::txn::RetCode;
// using claims::txn::FixTupleIngestReq;
// using claims::txn::Ingest;
// using claims::common::rSuccess;
// using claims::common::rLinkTmTimeout;
// using claims::common::rLinkTmFail;
// using claims::common::rBeginIngestTxnFail;
// using claims::common::rBeginQueryFail;
// using claims::common::rBeginCheckpointFail;
// using claims::common::rCommitIngestTxnFail;
// using claims::common::rCommitCheckpointFail;

string TxnClient::ip_ = kTxnIp;
int TxnClient::port_ = kTxnPort;
caf::actor TxnClient::proxy_;
RetCode TxnClient::Init(string ip, int port) {
  ip_ = ip;
  port_ = port;
  CAFSerConfig();
  try {
    proxy_ = caf::io::remote_actor(ip_, port);
  } catch (...) {
    //    return rLinkTmFail;
    return -1;
  }
  //  return rSuccess;
  return 0;
}

RetCode TxnClient::Debug(string flag) {
  RetCode ret = 0;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    DebugAtom::value, flag)
        .await([&ret](RetCode r) { ret = r; },
               caf::others >> []() { cout << " unkown message" << endl; },
               caf::after(seconds(kTimeout)) >> [&ret] {
                                                  ret = -1;
                                                  cout << "time out" << endl;
                                                });
  } catch (...) {
    cout << "link fail" << endl;
    return -1;
  }
  return ret;
}

RetCode TxnClient::BeginIngest(const FixTupleIngestReq& request,
                               Ingest& ingest) {
  //  RetCode ret = rSuccess;
  RetCode ret = 0;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    IngestAtom::value, request)
        .await([&](RetCode r, const Ingest& reply) {
                 ret = r;
                 ingest = reply;
               },
               [&](RetCode r) { ret = r; },
               caf::others >> []() { cout << " unkown message" << endl; },
               caf::after(seconds(kTimeout)) >> [&] {
                                                  ret = -1;
                                                  cout << "begin ingest timeout"
                                                       << endl;
                                                });
  } catch (...) {
    cout << "begin ingest link fail" << endl;
    //    return rLinkTmFail;
    return -1;
  }
  return ret;
}

RetCode TxnClient::CommitIngest(UInt64 ts) {
  //  RetCode ret = rSuccess;
  RetCode ret = 0;
  try {
    caf::scoped_actor self;

    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    CommitIngestAtom::value, ts)
        .await([&](RetCode r) { ret = r; },
               caf::others >> []() { cout << " unkown message" << endl; },
               caf::after(seconds(kTimeout)) >> [&] {
                                                  //           ret =
                                                  //           rLinkTmTimeout;
                                                  ret = -1;
                                                  cout
                                                      << "commit ingest timeout"
                                                      << endl;
                                                });
  } catch (...) {
    cout << "link fail" << endl;
    //    return rLinkTmFail;
    return -1;
  }
  return ret;
}

RetCode TxnClient::AbortIngest(UInt64 id) {
  //  RetCode ret = rSuccess;
  RetCode ret = 0;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    AbortIngestAtom::value, id)
        .await([&](RetCode r) { ret = r; },
               caf::others >> []() { cout << " unkown message" << endl; },
               caf::after(seconds(kTimeout)) >> [&] {
                                                  //                ret =
                                                  //                rLinkTmTimeout;
                                                  ret = -1;
                                                  cout << "abort ingest timeout"
                                                       << endl;
                                                });
  } catch (...) {
    cout << "link fail" << endl;
    //    return rLinkTmFail;
    return -1;
  }
  return ret;
}

RetCode TxnClient::BeginQuery(const QueryReq& request, Query& query) {
  //  RetCode ret = rSuccess;
  RetCode ret = 0;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    QueryAtom::value, request)
        .await([&query](const Query& q) { query = q; },
               caf::after(seconds(15)) >> [&ret] {
                                            ret = -1;
                                            cout << "begin query time out"
                                                 << endl;
                                          });
  } catch (...) {
    cout << "begin query link fail" << endl;
    //    return rLinkTmFail;
    return -1;
  }
  return ret;
}

RetCode TxnClient::CommitQuery(UInt64 ts) {
  RetCode ret = 0;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    CommitQueryAtom::value, ts)
        .await([&ret](RetCode r) { ret = r; },
               caf::after(seconds(kTimeout)) >> [&ret] {
                                                  ret = -1;
                                                  cout << "commit query timeout"
                                                       << endl;
                                                });
  } catch (...) {
    cout << "link to proxy fail in commitQuery" << endl;
  }
  return ret;
}

RetCode TxnClient::CommitCheckpoint(UInt64 ts, UInt64 part, UInt64 his_cp,
                                    UInt64 rt_cp) {
  RetCode ret = rSuccess;
  try {
    caf::scoped_actor self;
    self->sync_send(TxnServer::active_ ? TxnServer::proxy_ : proxy_,
                    CommitCPAtom::value, ts, part, his_cp, rt_cp)
        .await([&ret](RetCode r) { ret = r; },
               caf::after(seconds(kTimeout)) >> [&ret] {
                                                  //   ret =
                                                  //   rLinkTmTimeout;
                                                  ret = -1;
                                                  cout << "commit cp timeout"
                                                       << endl;
                                                });
  } catch (...) {
    cout << "link fail @ CommitCheckpoint" << endl;
    //    return rLinkTmFail;
    return -1;
  }
  return ret;
}
}  // namespace txn
}  // namespace claims
