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
 * /log/log.hpp
 *
 *  Created on: 2016年2月24日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef LOG_MANAGER_HPP_
#define LOG_MANAGER_HPP_
#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include <memory>
#include <regex>
#include "unistd.h"
#include "dirent.h"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "txn.hpp"

using std::string;
using std::map;
using std::thread;
using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::to_string;
using std::shared_ptr;


namespace claims{
namespace txn{




using UInt64 = unsigned long long;
using UInt32 = unsigned int;
const UInt64 kMaxLogSize = 16 * 1024 * 1024;
const string kTxnLogFileName = "txn_log_";
const string kDataLogFileName = "data_log_";
const int kTypeBegin = 1;
const int kTypeWrite = 2;
const int kTypeCommit = 3;
const int kTypeAbort = 4;
const int kTypeCP = 5;
const int kTypeData = 6;


class LogServer:public caf::event_based_actor {
 public:
  static RetCode Init(const string path = ".");
  static RetCode Append (const string & log);
  static RetCode Append (void * buffer, UInt64 size);
  static RetCode Refresh ();
  inline static string BeginLog(UInt64 Tid){
     return "begin<"+to_string(Tid) +">\n";
   }

   inline static string WriteLog(UInt64 id, UInt64 part, UInt64 pos,UInt64 offset) {
     return "write<"+to_string(id)+","+to_string(part)+","+
         to_string(pos)+","+to_string(offset)+">\n";
   }

   inline static string CommitLog(UInt64 id) {
     return "commit<"+to_string(id)+">\n";
   }
   inline static string AbortLog(UInt64 id) {
     return "abort<"+to_string(id)+">\n";
   }
   inline static string CheckpointLog(UInt64 part,UInt64 logic_cp, UInt64 phy_cp){
     return "checkpoint<"+to_string(part)
         +","+to_string(logic_cp)+","+to_string(phy_cp)+">\n";
   }
  inline static string DataLogPrefix(UInt64 part, UInt64 pos, UInt64 offset, UInt64 size ) {
    return "data<"+to_string(part)+","+to_string(pos)+
        ","+to_string(offset)+","+to_string(size)+">\n";
  }
  caf::behavior make_behavior();
  static caf::actor proxy_;
  static bool active_;
 private:
  static string file_path_;
  static FILE * file_handler_;
  static UInt64 file_size_;
  static UInt64 file_capacity_;
  static char * buffer_;
  static UInt64 buffer_size_;
  static UInt64 buffer_capacity_;

};


class LogClient{
 public:
  static RetCode Begin(UInt64 id);
  static RetCode Write(UInt64 id, UInt64 part, UInt64 pos, UInt64 offset);
  static RetCode Commit(UInt64 id);
  static RetCode Abort(UInt64 id);
  static RetCode Data(UInt64 part, UInt64 pos, UInt64 offset, void * buffer, UInt64 size);
  static RetCode Checkpoint(UInt64 part, UInt64 logic_cp, UInt64 phy_cp);
  static RetCode Refresh();

};


}
}
#endif //  LOG_MANAGER_HPP_ 
