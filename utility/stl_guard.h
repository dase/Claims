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
 * /Claims/utility/stl_guard.h
 *
 *  Created on: Apr 14, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef UTILITY_STL_GUARD_H_
#define UTILITY_STL_GUARD_H_
#include "../common/error_define.h"
#include "../common/memory_handle.h"
#include "../common/rename.h"

// a guard of STL container which stores pointer, like vector<int*>
template <typename T>
class STLGuardWithRetCode {
 public:
  STLGuardWithRetCode(T& t, RetCode& ret) : t_(t), ret_(ret) {}
  ~STLGuardWithRetCode() {
    if (claims::common::rSuccess != ret_) {
      for (auto it : t_) DELETE_PTR(it);
      t_.clear();
    }
  }

  NO_COPY_AND_ASSIGN(STLGuardWithRetCode);

 private:
  T& t_;
  RetCode& ret_;
};

template <typename T>
class TwoLayerSTLGuardWithRetCode {
 public:
  TwoLayerSTLGuardWithRetCode(T& t, RetCode& ret) : t_(t), ret_(ret) {}
  ~TwoLayerSTLGuardWithRetCode() {
    if (claims::common::rSuccess != ret_) {
      for (auto it1 : t_) {
        for (auto it2 : it1) {
          DELETE_PTR(it2);
        }
        it1.clear();
      }
      t_.clear();
    }
  }

  NO_COPY_AND_ASSIGN(TwoLayerSTLGuardWithRetCode);

 private:
  T& t_;
  RetCode& ret_;
};

template <typename T>
class ThreeLayerSTLGuardWithRetCode {
 public:
  ThreeLayerSTLGuardWithRetCode(T& t, RetCode& ret) : t_(t), ret_(ret) {}
  ~ThreeLayerSTLGuardWithRetCode() {
    if (claims::common::rSuccess != ret_) {
      for (auto it1 : t_) {
        for (auto it2 : it1) {
          for (auto it3 : it2) {
            DELETE_PTR(it3);
          }
          it2.clear();
        }
        it1.clear();
      }
      t_.clear();
    }
  }

  NO_COPY_AND_ASSIGN(ThreeLayerSTLGuardWithRetCode);

 private:
  T& t_;
  RetCode& ret_;
};

#endif  // UTILITY_STL_GUARD_H_
