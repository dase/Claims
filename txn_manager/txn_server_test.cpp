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
 * /txn/client.cpp
 *
 *  Created on: 2016年4月7日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */


#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>
#include <memory>
#include <map>
#include <thread>
#include <utility>
#include <unordered_map>
#include <tuple>
#include <time.h>
#include <chrono>
#include "unistd.h"
#include <sys/time.h>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
//#include "txn.hpp"
#include "txn_server.hpp"
#include "txn_client.hpp"
#include "txn_log.hpp"
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;
using std::unordered_map;
using std::to_string;
using std::function;
using std::sort;
using std::tuple;
using std::make_pair;
using std::make_tuple;
using std::get;
using std::shared_ptr;
using std::make_shared;
//using namespace claims::txn;

using UInt64 = unsigned long long;
using UInt32 = unsigned int;
using UInt16 = unsigned short;
using UInt8 = char;
using RetCode = int;
using OkAtom = caf::atom_constant<caf::atom("ok")>;
using IngestAtom = caf::atom_constant<caf::atom("ingest")>;
using QueryAtom = caf::atom_constant<caf::atom("query")>;
using FailAtom = caf::atom_constant<caf::atom("fail")>;
using QuitAtom = caf::atom_constant<caf::atom("quit")>;

class Foo {
 public:
  vector<UInt64> request1;
  unordered_map<UInt64, pair<UInt64, UInt64>> request2;
  vector<pair<int,int>> request3;
  void set_request1(const vector<UInt64> & req) { request1 = req;}
  void set_request2(const unordered_map<UInt64, pair<UInt64, UInt64>> & req) {
    request2 = req;
  }
  void set_request3(const vector<pair<int,int>>  &req) { request3 = req;}
  vector<UInt64> get_request1() const {return request1;}
  unordered_map<UInt64, pair<UInt64, UInt64>> get_request2() const {return request2;}
  vector<pair<int,int>> get_request3() const { return request3;}
};

inline bool operator == (const Foo & a, const Foo & b) {
  return a.request1 == b.request1 && a.request2 == b.request2;
}





class A{
 public:
  vector<int> list_ ;
  int c = 0;
  void set_list_(const vector<int> list) { list_ = list;}
  vector<int> get_list_() const { return list_;}
};
inline bool operator == (const A & a1, const A & a2) {
  return a1.list_ == a2.list_;
}

void ConfigA(){
  caf::announce<A>("A", make_pair(&A::get_list_, &A::set_list_));
}

class Core: public caf::event_based_actor {
 public:
  int id_;
  int v_;
  Core(int id):id_(id) {}
  caf::behavior make_behavior() override;
};
class Proxy: public caf::event_based_actor {
 public:
  caf::behavior make_behavior() override;
};
vector<caf::actor> cores;
caf::actor proxy;
caf::behavior Core::make_behavior() {
  return {
    [=](IngestAtom, int v)->int {
      v_ = v;
      //cout << "ingest:" << v_ << endl;
      return -v_;
     },
     [=](QueryAtom, shared_ptr<A> ptr)->A {
       //cout << id_ << endl;
       ptr->list_.push_back(-id_*id_);
      // ptr->list_.push_back(id_);
       if(id_ != cores.size() - 1) {
         this->forward_to(cores[id_ + 1]);
         return A();
         }
       else {
         return *ptr;
         };
       cout << "err" << endl;

       return A();
       },
     caf::others >> [] { cout << "core receive unkown message" << endl;}
  };
}

caf::behavior Proxy::make_behavior() {
  return {
    [=](IngestAtom, int v) {
        //cout << "begin ingest" << endl;
        this->forward_to(cores[v%cores.size()]);
     },
    [=](QueryAtom) {
       shared_ptr<A> ptr = make_shared<A>();
       this->current_message() = caf::make_message(QueryAtom::value, ptr);
       this->forward_to(cores[0]);
     },
     caf::others >> [] { cout << "proxy receive unkown message" << endl;}
  };
}
void task(int index){
for (auto i=0;i<index;i++) {
    caf::scoped_actor self;
    self->sync_send(proxy, IngestAtom::value, i).await(
        [=](int ret) { /*cout <<"receive:" << ret << endl;*/},
        caf::after(std::chrono::seconds(2)) >> [] {
            cout << "ingest time out" << endl;
         }
     );

  }
}


using claims::txn::FixTupleIngestReq;
using claims::txn::Ingest;
using claims::txn::TxnServer;
using claims::txn::TxnClient;
using claims::txn::LogServer;
using claims::txn::LogClient;
char buffer[3*1024+10];
void task2(int times){
for (auto i=0; i<times; i++) {
    FixTupleIngestReq req;
    Ingest ingest;
    req.InsertStrip(0, 50, 10);
    req.InsertStrip(1, 10 , 10);
    TxnClient::BeginIngest(req, ingest);
  //  cout << ingest.ToString() << endl;
   // LogClient::Data(0,0,0,buffer,1*1024);
    //LogClient::Data(0,0,0,buffer,1*1024);
    TxnClient::CommitIngest(ingest.ts_);
  }
}

int is_log = 0;
int main(int argc,char *argv[]){
  int con = stoi(string(argv[1]));
  int port = stoi(string(argv[2]));
  is_log = stoi(string(argv[3]));
  //memset(buffer,3*1024,'*');
  TxnServer::Init(con, port);
  //TxnClient::Init();
  if (is_log == 1)
    LogServer::Init("txn-log");
//  struct  timeval tv1, tv2;
//  vector<std::thread> threads;
//  int n,times;
//  cin >> n >> times;
//  for (auto i=0;i<n;i++)
//    threads.push_back(std::thread(task2, times));
//  gettimeofday(&tv1,NULL);
//  for (auto i=0;i<n;i++)
//    threads.push_back(std::thread(task2, times));
//  for (auto i=0;i<n;i++)
//    threads[i].join();
//  gettimeofday(&tv2,NULL);
//  cout << tv2.tv_sec - tv1.tv_sec << "-" << (tv2.tv_usec - tv1.tv_usec)/1000 <<endl;
  caf::await_all_actors_done();
  caf::shutdown();
}
