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
 * /Claims/exec_tracker/segment_exec_status.cpp
 *
 *  Created on: Apr 3, 2016
 *      Author: fzh
 *		   Email: fzhedu@gmail.com
 *
 * Description:
 *
 */

#include "./segment_exec_status.h"
#include <glog/logging.h>
#include <string>
#include <iostream>
#include "../exec_tracker/stmt_exec_tracker.h"
#include "../Environment.h"
#include "caf/io/all.hpp"
using caf::io::remote_actor;
using std::string;
using std::endl;
namespace claims {

SegmentExecStatus::SegmentExecStatus(NodeSegmentID node_segment_id,
                                     unsigned int coor_node_id)
    : node_segment_id_(node_segment_id),
      coor_node_id_(coor_node_id),
      exec_info_("ok"),
      exec_status_(ExecStatus::kOk),
      ret_code_(0),
      logic_time_(0),
      stop_report_(false) {
  //  RegisterToTracker();
  coor_actor_ =
      Environment::getInstance()->get_slave_node()->GetNodeActorFromId(
          coor_node_id);
}
SegmentExecStatus::SegmentExecStatus(NodeSegmentID node_segment_id)
    : node_segment_id_(node_segment_id),
      exec_info_("ok"),
      exec_status_(ExecStatus::kOk),
      ret_code_(0),
      stop_report_(false),
      logic_time_(Environment::getInstance()
                      ->get_stmt_exec_tracker()
                      ->get_logic_time()) {}
SegmentExecStatus::~SegmentExecStatus() {
  //  ostringstream exec_info;
  //  exec_info << "query (" << node_segment_id_.first << " , "
  //            << node_segment_id_.second / kMaxNodeNum << " ) at node "
  //            << node_segment_id_.second % kMaxNodeNum << " execution
  //            succeed";
  //  UpdateStatus(SegmentExecStatus::ExecStatus::kDone, exec_info.str(), 0,
  //  true);
  //  ReportStatus(SegmentExecStatus::ExecStatus::kDone, exec_info.str());
  //  UnRegisterFromTracker();
}

RetCode SegmentExecStatus::CancelSegExec() {
  //  lock_.acquire();
  exec_status_ = kCancelled;
  //  lock_.release();
  LOG(INFO) << node_segment_id_.first << " , " << node_segment_id_.second
            << " need be cancelled!" << endl;
  return rSuccess;
}
// make sure that kCancel or kDone is the last message transfer to stmt
RetCode SegmentExecStatus::ReportStatus(ExecStatus exec_status,
                                        string exec_info) {
  if (kCancelled == exec_status_ || stop_report_) {
    lock_.release();
    return rSuccess;
  }
  //  lock_.acquire();
  try {
    caf::scoped_actor self;
    //    coor_actor_ = remote_actor(coor_addr_.first, coor_addr_.second);

    self->sync_send(coor_actor_, ReportSegESAtom::value, node_segment_id_,
                    (int)exec_status, exec_info)
        .await(

            [=](OkAtom) {
              if (kCancelled == exec_status || kDone == exec_status) {
                stop_report_ = true;
              }
              LOG(INFO) << node_segment_id_.first << " , "
                        << node_segment_id_.second
                        << " report0: " << exec_status << " , " << exec_info;
            },
            [=](CancelPlanAtom) {
              stop_report_ = true;
              CancelSegExec();
            },
            caf::after(std::chrono::seconds(kTimeout)) >>
                [&]() {
                  LOG(WARNING)
                      << "segment report status timeout and cancel self!";
                  CancelSegExec();
                });

  } catch (caf::network_error& e) {
    LOG(WARNING) << node_segment_id_.first << " , " << node_segment_id_.second
                 << " cann't connect to node  ( " << coor_node_id_
                 << " ) when report status";
  }
  lock_.release();

  // for making sure kError or kDone be the last message sended to remote
  //  if (ExecStatus::kError == exec_status_ || ExecStatus::kDone ==
  //  exec_status_) {
  //    lock_.acquire();
  //    exec_status_ = kCancelled;
  //    lock_.release();
  //  }
  return rSuccess;
}
RetCode SegmentExecStatus::ReportStatus() {
  // if this segment is cancelled, needn't report status
  lock_.acquire();
  if (kCancelled == exec_status_ || stop_report_) {
    lock_.release();
    return rSuccess;
  }
  ExecStatus exec_status = exec_status_;
  string exec_info = exec_info_;
  lock_.release();
  try {
    caf::scoped_actor self;
    self->sync_send(coor_actor_, ReportSegESAtom::value, node_segment_id_,
                    (int)exec_status, exec_info)
        .await(

            [=](OkAtom) {
              if (kCancelled == exec_status || kDone == exec_status) {
                stop_report_ = true;
              }
              LOG(INFO) << node_segment_id_.first << " , "
                        << node_segment_id_.second
                        << " report : " << exec_status << " , " << exec_info;
            },
            [=](CancelPlanAtom) {
              stop_report_ = true;
              CancelSegExec();
            },
            caf::after(std::chrono::seconds(kTimeout)) >>
                [&]() {
                  LOG(WARNING)
                      << "segment report status timeout and cancel self!";
                  CancelSegExec();
                });

  } catch (caf::network_error& e) {
    LOG(WARNING) << node_segment_id_.first << " , " << node_segment_id_.second
                 << " cann't connect to node ( " << coor_node_id_
                 << " ) when report status";
  }

  // for making sure kError or kDone be the last message sended to remote
  //  if (ExecStatus::kError == exec_status || ExecStatus::kDone == exec_status)
  //  {
  //    lock_.acquire();
  //    exec_status_ = kCancelled;
  //    lock_.release();
  //  }
  return rSuccess;
}
bool SegmentExecStatus::UpdateStatus(ExecStatus exec_status, string exec_info,
                                     u_int64_t logic_time, bool need_report) {
  lock_.acquire();
  if (exec_status_ == ExecStatus::kCancelled) {
    LOG(INFO) << node_segment_id_.first << " , " << node_segment_id_.second
              << " update status failed!";
    lock_.release();
    return false;
  } else if (ExecStatus::kOk == exec_status_) {
    logic_time_ = logic_time;
    exec_status_ = exec_status;
    exec_info_ = exec_info;
    LOG(INFO) << node_segment_id_.first << " , " << node_segment_id_.second
              << " update logic_time= " << logic_time_
              << " exec_status_= " << exec_status_;
    lock_.release();
    if (need_report) {
      lock_.acquire();
      ReportStatus(exec_status, exec_info);
      // ReportStatus();
    }
  } else {
    lock_.release();
    LOG(WARNING) << "segment's status shouldn't be updated!!";
  }
  return true;
}
RetCode SegmentExecStatus::RegisterToTracker() {
  return Environment::getInstance()->get_segment_exec_tracker()->RegisterSegES(
      node_segment_id_, this);
}

RetCode SegmentExecStatus::UnRegisterFromTracker() {
  while (true) {
    if (exec_status_ == kOk) {
      LOG(INFO) << node_segment_id_.first << " , " << node_segment_id_.second
                << " " << exec_status_ << " should be 3";
      UpdateStatus(SegmentExecStatus::ExecStatus::kDone, "finished", 0, true);
    } else {
      break;
    }
  }
  return Environment::getInstance()
      ->get_segment_exec_tracker()
      ->UnRegisterSegES(node_segment_id_);
}
bool SegmentExecStatus::HaveErrorCase(u_int64_t logic_time) {
  LOG(INFO) << node_segment_id_.first << " , " << node_segment_id_.second << " "
            << exec_status_ << " " << logic_time << " - " << logic_time_
            << " = " << logic_time - logic_time_;
  // if the exection status is normal, but logic time falls behind 4s
  return (exec_status_ == kOk) && (logic_time - logic_time_ > 4);
}

}  // namespace claims
