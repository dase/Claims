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
 * /Claims/utility/resource_guard.h
 *
 *  Created on: Apr 19, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef UTILITY_RESOURCE_GUARD_H_
#define UTILITY_RESOURCE_GUARD_H_

#include "../common/error_define.h"
#include "../common/memory_handle.h"
#include "../common/rename.h"

// auto release memory stored in t_.
// Attention: if 't_' is void*, the typename T is void !!!
template <typename T>
class MemoryGuard {
 public:
  explicit MemoryGuard(T*& t) : t_(t) {}
  ~MemoryGuard() { DELETE_PTR(t_); }

  NO_COPY_AND_ASSIGN(MemoryGuard);

 private:
  T*& t_;
};

// if the memory is still in use after function return, please use
// MemoryGuardWithRetCode
// only some error occurs in function, the memory will be auto-free
// Attention: if some error occurs, the 'ret' must be set !!!
template <typename T>
class MemoryGuardWithRetCode {
 public:
  MemoryGuardWithRetCode(T*& t, RetCode& ret) : t_(t), ret_(ret) {}
  ~MemoryGuardWithRetCode() {
    if (claims::common::rSuccess != ret_) DELETE_PTR(t_);
  }

  NO_COPY_AND_ASSIGN(MemoryGuardWithRetCode);

 private:
  T*& t_;
  RetCode& ret_;
};

// a guard of STL container which stores pointer, like vector<int*>
// if ret != rSuccess, the memory will be released
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

#endif  // UTILITY_RESOURCE_GUARD_H_
