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
 * /Claims/loader/load_packet.h
 *
 *  Created on: Apr 17, 2016
 *      Author: yukai
 *		   Email: yukai2014@gmail.com
 *
 * Description:
 *
 */

#ifndef LOADER_LOAD_PACKET_H_
#define LOADER_LOAD_PACKET_H_
#include "../common/error_define.h"
#include "../txn_manager/txn.hpp"

namespace claims {
namespace loader {

using IpPortAtom = caf::atom_constant<caf::atom("ip_port")>;
using LoadPacketAtom = caf::atom_constant<caf::atom("load_pack")>;
using LoadAckAtom = caf::atom_constant<caf::atom("load_ack")>;
using RegNodeAtom = caf::atom_constant<caf::atom("reg_node")>;
using BindPartAtom = caf::atom_constant<caf::atom("bind_part")>;
using OkAtom = caf::atom_constant<caf::atom("ok")>;
using AddBlockAtom = caf::atom_constant<caf::atom("add_block")>;

/**************  LoadPacket format  *****************/
/** field             type          length **********/
/****************************************************/
/** transaction_id    uint64_t      8              **/
/** global_part_id    uint64_t      8              **/
/** position          uint64_t      8              **/
/** offset            uint64_t      8              **/
/** date_length       uint64_t      8              **/
/** data              void*         data_length    **/
/****************************************************/
struct LoadPacket {
 public:
  LoadPacket() {}
  LoadPacket(int socket_fd, const uint64_t txn_id, const uint64_t g_part_id,
             uint64_t pos, uint64_t offset, uint64_t data_length,
             void* data_buffer)
      : txn_id_(txn_id),
        global_part_id_(g_part_id),
        pos_(pos),
        offset_(offset),
        data_buffer_(data_buffer),
        data_length_(data_length),
        socket_fd_(socket_fd) {}
  ~LoadPacket();
  RetCode Serialize();

  RetCode Deserialize(const void* const head_buffer,
                      const void* const data_buffer);

 public:
  static const int kHeadLength = 5 * sizeof(uint64_t);
  const void* getDataBuffer() { return data_buffer_; }
  void setDataBuffer(void* data_buffer) { data_buffer_ = data_buffer; }

 public:
  uint64_t txn_id_;
  uint64_t global_part_id_;
  uint64_t pos_;
  uint64_t offset_;
  uint64_t data_length_;
  void* data_buffer_;

  int socket_fd_ = -1;
  uint64_t packet_length_ = 0;
  void* packet_buffer_ = NULL;
};

inline bool operator==(const LoadPacket& a, const LoadPacket& b) {
  return a.txn_id_ == b.txn_id_ && a.global_part_id_ == b.global_part_id_ &&
         a.pos_ == b.pos_;
}

} /* namespace loader */
} /* namespace claims */

#endif  // LOADER_LOAD_PACKET_H_
