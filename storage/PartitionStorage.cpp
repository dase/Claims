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
 * /CLAIMS/storage/PartitionStorage.cpp
 *
 *  Created on: NOV 14 ,2013
 *  Modified on: NOV 28, 2015
 *      Author: Hanzhang, wangli
 *       Email:
 *
 * Description:
 *
 */

#include "PartitionStorage.h"

#include <assert.h>
#include <vector>
#include <string>
#include "../common/error_define.h"
// #include "../Debug.h"
#include "./MemoryManager.h"
#include "../common/memory_handle.h"
#include "../Config.h"
#include "../Resource/BufferManager.h"
#include "../storage/BlockManager.h"
#include "../common/file_handle/file_handle_imp.h"
#include "../common/file_handle/hdfs_file_handle_imp.h"
#include "../common/file_handle/file_handle_imp_factory.h"
#include "../utility/resource_guard.h"
using claims::common::rSuccess;
using claims::common::FileHandleImpFactory;
using claims::common::FileHandleImp;
using claims::common::kHdfs;
using claims::common::kDisk;
using claims::utility::LockGuard;
using claims::common::rGetFileHandleFail;
using claims::common::rGetMemoryHandleFail;

ofstream logfile;
ofstream qylog;
int qylog_count = 0;
ofstream mvlog;
int mvlog_count = 0;

/**
 * According to number_of_chunks, construct chunk from partition and add into
 * the chunk_list_. Meantime, you can get specific information about chunk. when
 * number_of_chunks more than storage_max_bugdege, you should choose the random
 * way to remove the chunk which in the memory. or not, choose LRU.
 */
PartitionStorage::PartitionStorage(const PartitionID& partition_id,
                                   const unsigned& number_of_chunks,

                                   const StorageLevel& storage_level)
    : partition_id_(partition_id),
      number_of_chunks_(number_of_chunks),
      number_of_rt_chunks_(0),
      desirable_storage_level_(storage_level) {
  if (number_of_chunks_ * CHUNK_SIZE / 1024 / 1024 >
      BufferManager::getInstance()->getStorageMemoryBudegeInMilibyte() *
          Config::memory_utilization / 100)
    MemoryChunkStore::GetInstance()->SetFreeAlgorithm(0);
  else
    MemoryChunkStore::GetInstance()->SetFreeAlgorithm(1);
  /* for (unsigned i = 0; i < number_of_chunks_; i++) {
     chunk_list_.push_back(new ChunkStorage(
         ChunkID(partition_id_, i), BLOCK_SIZE, desirable_storage_level_));
     rt_chunk_list_.push_back(new ChunkStorage(
         ChunkID(partition_id_, i, true), BLOCK_SIZE,
   desirable_storage_level_));
   }*/
  // CheckAndAppendChunkList(number_of_chunks_, false);
  // CheckAndAppendChunkList(number_of_chunks_, true);
  // cout << "*******chunk_list_" << chunk_list_.size() << endl;
  // cout << "open ps log" << endl;
  // if (!logfile.is_open()) logfile.open("ps_log.txt");
  if (!qylog.is_open()) qylog.open("qy_log.txt");
  if (!mvlog.is_open()) mvlog.open("mv_log.txt");
}

PartitionStorage::~PartitionStorage() {
  for (unsigned i = 0; i < chunk_list_.size(); i++) {
    DELETE_PTR(chunk_list_[i]);
  }
  chunk_list_.clear();
  // cout << "close ps log" << endl;
  // logfile.close();
}

void PartitionStorage::AddNewRTChunk() {
  LockGuard<Lock> guard(write_lock_);
  number_of_rt_chunks_++;
}

void PartitionStorage::AddNewHisChunk() {
  LockGuard<Lock> guard(write_lock_);
  number_of_chunks_++;
}

RetCode PartitionStorage::AddHisChunkWithMemoryApply(
    unsigned expected_number_of_chunks, const StorageLevel& storage_level) {
  RetCode ret = rSuccess;
  LockGuard<Lock> guard(write_lock_);
  // cout << "1..." << endl;
  if (chunk_list_.size() >= expected_number_of_chunks) return ret;
  DLOG(INFO) << "now chunk number:" << number_of_chunks_
             << ". expected chunk num:" << expected_number_of_chunks;
  if (chunk_list_.size() >= expected_number_of_chunks) return ret;
  for (unsigned i = chunk_list_.size(); i < expected_number_of_chunks; i++) {
    ChunkStorage* chunk =
        new ChunkStorage(ChunkID(partition_id_, i), BLOCK_SIZE, storage_level);
    chunk_list_.push_back(chunk);
    EXEC_AND_DLOG(ret, chunk_list_[i]->ApplyMemory(),
                  "applied memory for chunk(" << partition_id_.getName() << ","
                                              << i << ")",
                  "failed to apply memory for chunk(" << partition_id_.getName()
                                                      << "," << i << ")");
    assert(ret == rSuccess);
    // cout << "2...." << ret << endl;
  }

  number_of_chunks_ = expected_number_of_chunks;
  assert(chunk_list_.size() == number_of_chunks_);

  return ret;
}

RetCode PartitionStorage::AddRtChunkWithMemoryApply(
    unsigned expected_number_of_chunks, const StorageLevel& storage_level) {
  RetCode ret = rSuccess;
  LockGuard<Lock> guard(write_lock_);
  // cout << "******-1*****" << endl;
  if (number_of_rt_chunks_ >= expected_number_of_chunks) return ret;
  DLOG(INFO) << "now rt chunk number:" << number_of_rt_chunks_
             << ". expected rt chunk num:" << expected_number_of_chunks;
  // cout << "******0*****" << endl;

  if (number_of_rt_chunks_ >= expected_number_of_chunks) return ret;

  for (unsigned i = number_of_rt_chunks_; i < expected_number_of_chunks; i++) {
    ChunkStorage* chunk = new ChunkStorage(ChunkID(partition_id_, i, true),
                                           BLOCK_SIZE, storage_level);
    rt_chunk_list_.push_back(chunk);
    EXEC_AND_DLOG(ret, chunk->ApplyMemory(), "applied memory for rt chunk("
                                                 << partition_id_.getName()
                                                 << "," << i << ")",
                  "failed to apply memory for rt chunk("
                      << partition_id_.getName() << "," << i << ")");
  }

  number_of_rt_chunks_ = expected_number_of_chunks;
  assert(rt_chunk_list_.size() == number_of_rt_chunks_);

  return ret;
}

/**
 * when appending data, the last chunk may be dirty so set storage level as HDFS
 * to make sure the data will be reload from HDFS. actually, DISK is not used,
 * only HDFS and MEMORY is used.
 */
void PartitionStorage::UpdateChunksWithInsertOrAppend(
    const PartitionID& partition_id, const unsigned& number_of_chunks,
    const StorageLevel& storage_level) {
  LockGuard<Lock> guard(write_lock_);
  if (!chunk_list_.empty()) {
    MemoryChunkStore::GetInstance()->ReturnChunk(
        chunk_list_.back()->GetChunkID());
    chunk_list_.back()->SetCurrentStorageLevel(HDFS);
  }
  for (unsigned i = number_of_chunks_; i < number_of_chunks; i++) {
    chunk_list_.push_back(
        new ChunkStorage(ChunkID(partition_id, i), BLOCK_SIZE, storage_level));
    /* rt_chunk_list_.push_back(new ChunkStorage(ChunkID(partition_id, i, true),
                                               BLOCK_SIZE, storage_level));*/
  }
  number_of_chunks_ = number_of_chunks;
}

/**
 * By searching in chunk_list_ to get chunk address(physical information), and
 * free the memory. This function is a logical process of delete the chunk, and
 * call back actual method.
 */
void PartitionStorage::RemoveAllChunks(const PartitionID& partition_id) {
  LockGuard<Lock> guard(write_lock_);
  if (!chunk_list_.empty()) {
    vector<ChunkStorage*>::iterator iter = chunk_list_.begin();
    MemoryChunkStore* mcs = MemoryChunkStore::GetInstance();
    for (; iter != chunk_list_.end(); iter++) {
      mcs->ReturnChunk((*iter)->GetChunkID());
    }
    chunk_list_.clear();
    number_of_chunks_ = 0;
  }
}

PartitionStorage::PartitionReaderIterator*
PartitionStorage::CreateReaderIterator() {
  return new PartitionReaderIterator(this);
}
PartitionStorage::PartitionReaderIterator*
PartitionStorage::CreateAtomicReaderIterator() {
  return new AtomicPartitionReaderIterator(this);
}

PartitionStorage::PartitionReaderIterator::PartitionReaderIterator(
    PartitionStorage* partition_storage)
    : ps_(partition_storage), chunk_cur_(0), chunk_it_(NULL) {}

PartitionStorage::PartitionReaderIterator::~PartitionReaderIterator() {}

ChunkReaderIterator* PartitionStorage::PartitionReaderIterator::NextChunk() {
  //  LockGuard<Lock> guard(ps_->write_lock_);
  if (chunk_cur_ < ps_->number_of_chunks_)
    return ps_->chunk_list_[chunk_cur_++]->CreateChunkReaderIterator();
  else
    return NULL;
}

PartitionStorage::AtomicPartitionReaderIterator::
    ~AtomicPartitionReaderIterator() {}

ChunkReaderIterator*
PartitionStorage::AtomicPartitionReaderIterator::NextChunk() {
  LockGuard<Lock> guard(ps_->write_lock_);
  if (chunk_cur_ < ps_->number_of_chunks_)
    return ps_->chunk_list_[chunk_cur_++]->CreateChunkReaderIterator();
  else
    return NULL;
}

bool PartitionStorage::PartitionReaderIterator::NextBlock(
    BlockStreamBase*& block) {
  assert(false);
  if (chunk_it_ > 0 && chunk_it_->NextBlock(block)) {
    return true;
  } else {
    if ((chunk_it_ = NextChunk()) > 0) {
      return NextBlock(block);
    } else {
      return false;
    }
  }
}

bool PartitionStorage::AtomicPartitionReaderIterator::NextBlock(
    BlockStreamBase*& block) {
  lock_.acquire();
  ChunkReaderIterator::block_accessor* ba = NULL;
  if (NULL != chunk_it_ && chunk_it_->GetNextBlockAccessor(ba)) {
    lock_.release();
    ba->GetBlock(block);
    if (NULL != ba) {
      delete ba;
      ba = NULL;
    }
    return true;
  } else {
    if (NULL != chunk_it_) {
      delete chunk_it_;
      chunk_it_ = NULL;
    }
    if ((chunk_it_ = NextChunk()) > 0) {
      lock_.release();
      return NextBlock(block);
    } else {
      lock_.release();
      return false;
    }
  }
}

PartitionStorage::TxnPartitionReaderIterator::TxnPartitionReaderIterator(
    PartitionStorage* partition_storage, uint64_t his_cp,
    const vector<PStrip>& rt_strip_list)
    : PartitionReaderIterator(partition_storage),
      last_his_block_(his_cp / BLOCK_SIZE),
      block_cur_(0),
      chunk_cur_(-1),
      rt_block_index_(0),
      rt_chunk_cur_(-1),
      rt_chunk_it_(nullptr) {
  for (auto& strip : rt_strip_list) {
    auto begin = strip.first;
    auto end = begin + strip.second;
    while (begin < end) {
      auto block = begin / BLOCK_SIZE;
      auto len = (static_cast<UInt64>(block) + 1) * BLOCK_SIZE <= end
                     ? (static_cast<UInt64>(block) + 1) * BLOCK_SIZE - begin
                     : end - begin;
      rt_strip_list_.push_back(PStrip(begin, len));
      begin += len;
    }
  }
  cout << "snapshot size:" << rt_strip_list_.size() << endl;
  /*  string str = "rt:";
    for (auto& strip : rt_strip_list_) {
      str += "<" + to_string(strip.first) + "," + to_string(strip.second) + ">";
    }
    cout << str << endl;*/
  qylog << "*********query:" << qylog_count++ << "***********" << endl;
  qylog << "last_his_block:" << last_his_block_ << endl;
}

PartitionStorage::TxnPartitionReaderIterator::~TxnPartitionReaderIterator() {
  for (auto block : rt_block_buffer_) free(block);
}

bool PartitionStorage::TxnPartitionReaderIterator::NextBlock(
    BlockStreamBase*& block) {
  LockGuard<Lock> guard(lock_);
  HdfsInMemoryChunk chunk;
  ChunkReaderIterator::block_accessor* ba = nullptr;
  if (block_cur_ < last_his_block_) {  // scan historical data
    int64_t chunk_cur = block_cur_ / (CHUNK_SIZE / BLOCK_SIZE);
    if (chunk_cur != chunk_cur_) {  // update chunk_it_
      chunk_cur_ = chunk_cur;
      ps_->CheckAndAppendChunkList(chunk_cur_ + 1, false);
      if (chunk_it_ != nullptr) delete chunk_it_;
      chunk_it_ = ps_->chunk_list_[chunk_cur_]->CreateChunkReaderIterator();
    }
    assert(chunk_it_ != nullptr);
    chunk_it_->GetNextBlockAccessor(ba);
    if (ba == nullptr) {
      if (chunk_it_ != nullptr) delete chunk_it_;
      assert(false);
      return false;
    } else {
      assert(ba != nullptr);
      ba->GetBlock(block);
      delete ba;
      ba = nullptr;
      block_cur_++;
      /*      logfile << "H<" << chunk_it_->chunk_id_.chunk_off << "," <<
         block_cur_
                    << "," << 0 << "," << block->getTuplesInBlock() << ">" <<
         endl;*/
      if (!block->Full())
        qylog << "H<cid:" << chunk_cur_ << ",bid:" << block_cur_ - 1
              << ",addr:" << block->getBlock()
              << ",sizes:" << block->getTuplesInBlock() << ">" << endl;
      return true;
    }
  } else if (rt_block_index_ < rt_strip_list_.size()) {  // scan real-time data
    auto pos = rt_strip_list_[rt_block_index_].first;
    auto offset_in_block = pos - (pos / BLOCK_SIZE) * BLOCK_SIZE;
    assert(offset_in_block >= 0);
    auto len = rt_strip_list_[rt_block_index_].second;
    auto rt_block_cur = pos / BLOCK_SIZE;
    auto rt_chunk_cur = pos / CHUNK_SIZE;
    // cout << "visit rt:<" << rt_chunk_cur << "," << rt_block_cur << ">" <<
    // endl;
    if (rt_chunk_cur != rt_chunk_cur_) {  // move to new rt chunk
      rt_chunk_cur_ = rt_chunk_cur;
      rt_block_cur_ = rt_chunk_cur_ * (CHUNK_SIZE / BLOCK_SIZE);
      ps_->CheckAndAppendChunkList(rt_chunk_cur_ + 1, true);
      if (rt_chunk_it_ != nullptr) delete rt_chunk_it_;
      rt_chunk_it_ =
          ps_->rt_chunk_list_[rt_chunk_cur_]->CreateChunkReaderIterator();
      assert(rt_chunk_it_ != nullptr);
    }
    do {  // move to rt_block_cur
      if (ba != nullptr) delete ba;
      rt_chunk_it_->GetNextBlockAccessor(ba);
      rt_block_cur_++;
    } while (rt_block_cur_ <= rt_block_cur);

    if (len == BLOCK_SIZE) {  // directly return pointer
      ba->GetBlock(block);
      /*      logfile << "R<" << rt_chunk_it_->chunk_id_.chunk_off << ","
                    << rt_block_cur << "," << offset_in_block << ","
                    << block->getTuplesInBlock() << ">" << endl;*/
      if (!block->Full())
        qylog << "warning: R<id:" << rt_block_cur - 1
              << ",sizes:" << block->getTuplesInBlock() << ">" << endl;
    } else {
      auto tuple_size =
          reinterpret_cast<BlockStreamFix*>(block)->getTupleSize();

      if ((pos + len) % BLOCK_SIZE == 0) len -= sizeof(unsigned);
      assert(pos / BLOCK_SIZE == (pos + len) / BLOCK_SIZE);
      auto tuple_count = len / tuple_size;

      ba->GetBlock(block);
      auto des_addr = reinterpret_cast<void*>(malloc(BLOCK_SIZE));
      auto scr_addr = block->getBlockDataAddress() + offset_in_block;
      memcpy(des_addr, scr_addr, len);
      *reinterpret_cast<unsigned*>(des_addr + BLOCK_SIZE - sizeof(unsigned)) =
          tuple_count;
      reinterpret_cast<BlockStreamFix*>(block)->setBlockDataAddress(des_addr);
      reinterpret_cast<BlockStreamFix*>(block)->setTuplesInBlock(tuple_count);
      rt_block_buffer_.push_back(des_addr);
      /* logfile << "R<" << rt_block_cur << "," << offset_in_block << ","
               << block->getTuplesInBlock() << ">" << endl;*/
      qylog << "R<id:" << rt_block_cur - 1
            << ",sizes:" << block->getTuplesInBlock() << ">" << endl;
    }
    auto count =
        *reinterpret_cast<unsigned int*>(block->getBlockDataAddress() +
                                         BLOCK_SIZE - sizeof(unsigned int));
    delete ba;
    ba = nullptr;
    rt_block_index_++;
    return true;
  }
  /* logfile << "********* query end **********" << endl;*/
  return false;
}
void PartitionStorage::CheckAndAppendChunkList(unsigned number_of_chunk,
                                               bool is_rt) {
  LockGuard<Lock> guard(write_lock_);
  if (!is_rt) {
    for (auto size = chunk_list_.size(); size < number_of_chunk; size++)
      chunk_list_.push_back(new ChunkStorage(
          ChunkID(partition_id_, size), BLOCK_SIZE, desirable_storage_level_));
  } else {
    for (auto size = rt_chunk_list_.size(); size < number_of_chunk; size++)
      rt_chunk_list_.push_back(
          new ChunkStorage(ChunkID(partition_id_, size, true), BLOCK_SIZE,
                           desirable_storage_level_));
  }
  assert(desirable_storage_level_ == MEMORY);
}

UInt64 PartitionStorage::MergeToHis(UInt64 old_his_cp,
                                    const vector<PStrip>& strip_list) {
  auto new_his_cp = old_his_cp;
  auto table_id = partition_id_.projection_id.table_id;
  auto proj_id = partition_id_.projection_id.projection_off;
  Schema* schema = Catalog::getInstance()
                       ->getTable(table_id)
                       ->getProjectoin(proj_id)
                       ->getSchema();
  MemoryGuard<Schema> schema_guard(schema);
  auto tuple_size = schema->getTupleMaxSize();
  HdfsInMemoryChunk chunk_rt, chunk_his;
  // if (strip_list.size() > 0) cout << "{before merge" << endl;
  if (strip_list.size() > 0) {
    mvlog << "*********mov:" << mvlog_count++ << "************" << endl;
    mvlog << "plan:";
    for (auto& strip : strip_list)
      mvlog << "<" << strip.first / BLOCK_SIZE << ","
            << strip.first % BLOCK_SIZE << "=>"
            << (strip.first + strip.second) / BLOCK_SIZE << ","
            << (strip.first + strip.second) % BLOCK_SIZE << ">";
    mvlog << endl;
  }
  for (auto& strip : strip_list) {
    auto begin = strip.first;
    auto end = strip.first + strip.second;
    while (begin < end) {
      auto move = BLOCK_SIZE - (begin + BLOCK_SIZE) % BLOCK_SIZE;
      if (move > end - begin) move = end - begin;
      // update historical chunk cur
      AddHisChunkWithMemoryApply(new_his_cp / CHUNK_SIZE + 1, MEMORY);
      // be carry! the chunk is not begin/CHUN_SIZE
      if (!BlockManager::getInstance()->getMemoryChunkStore()->GetChunk(
              chunk_list_[new_his_cp / CHUNK_SIZE]->GetChunkID(), chunk_his)) {
        // assert(false && begin && begin / CHUNK_SIZE);
      }
      // update real time chunk cur
      // real-time chunk need't check
      if (!BlockManager::getInstance()->getMemoryChunkStore()->GetChunk(
              rt_chunk_list_[begin / CHUNK_SIZE]->GetChunkID(), chunk_rt)) {
        assert(false);
        return old_his_cp;
      }
      // each step move just one full block or even partly block
      auto tail_offset =
          (new_his_cp + BLOCK_SIZE) % CHUNK_SIZE - sizeof(unsigned);
      auto his_addr = chunk_his.hook + new_his_cp % CHUNK_SIZE;
      auto rt_addr = chunk_rt.hook + begin % CHUNK_SIZE;

      if (move == BLOCK_SIZE) {  // full block
        memcpy(his_addr, rt_addr, BLOCK_SIZE);
        /*        logfile << "full<" << begin / BLOCK_SIZE << "," << begin %
           BLOCK_SIZE
                        << ","
                        << *reinterpret_cast<unsigned*>(rt_addr + BLOCK_SIZE -
                                                        sizeof(unsigned)) << ">
           => <"
                        << new_his_cp / BLOCK_SIZE << "," << new_his_cp %
           BLOCK_SIZE
                        << ","
                        << *reinterpret_cast<unsigned*>(
                               his_addr + BLOCK_SIZE - sizeof(unsigned)) << ">"
           << endl;*/
      } else {  // partly block
        auto real_move =
            (begin + move) % BLOCK_SIZE != 0 ? move : move - sizeof(unsigned);
        auto tuple_count = real_move / tuple_size;
        memcpy(his_addr, rt_addr, real_move);
        *reinterpret_cast<unsigned*>(his_addr + BLOCK_SIZE - sizeof(unsigned)) =
            tuple_count;

        /*        logfile << "part<" << begin / BLOCK_SIZE << "," << begin %
           BLOCK_SIZE
                        << "," << tuple_count << "> => <" << new_his_cp /
           BLOCK_SIZE
                        << "," << new_his_cp % BLOCK_SIZE << ","
                        << *reinterpret_cast<unsigned*>(
                               his_addr + BLOCK_SIZE - sizeof(unsigned)) << ">"
           << endl;*/
        mvlog << begin / BLOCK_SIZE << "=>" << new_his_cp / BLOCK_SIZE << ":"
              << tuple_count << endl;
      }

      begin += move;
      new_his_cp += BLOCK_SIZE;
    }
  }

  // if (strip_list.size() > 0) logfile << "********************" << endl;

  return new_his_cp;
}

RetCode PartitionStorage::Persist(UInt64 old_his_cp, UInt64 new_his_cp) {
  FileHandleImp* file_handle = nullptr;
  MemoryGuard<FileHandleImp> file_handle_guard(file_handle);
  if (!Config::local_disk_mode)
    file_handle = FileHandleImpFactory::Instance().CreateFileHandleImp(
        kHdfs, partition_id_.getPathAndName());
  else
    file_handle = FileHandleImpFactory::Instance().CreateFileHandleImp(
        kDisk, partition_id_.getPathAndName());
  if (file_handle == nullptr) return rGetFileHandleFail;
  HdfsInMemoryChunk chunk_his;
  auto begin = old_his_cp;
  auto end = new_his_cp;
  while (begin < end) {
    if (!BlockManager::getInstance()->getMemoryChunkStore()->GetChunk(
            ChunkID(partition_id_, begin / CHUNK_SIZE), chunk_his))
      return rGetMemoryHandleFail;
    auto move = CHUNK_SIZE - (begin + CHUNK_SIZE) % CHUNK_SIZE;
    if (begin + move > end) move = end - begin;
    file_handle->Append(chunk_his.hook + begin % CHUNK_SIZE, move);
    /*    cout << "move chunk:" << begin / CHUNK_SIZE << "," << begin %
       CHUNK_SIZE
             << " step:" << move << endl;*/
    begin += move;
  }
  return rSuccess;
}

bool PartitionStorage::PersistHDFS(UInt64 old_his_cp, UInt64 new_his_cp) {
  /*
   * ToDo Truncate need to be implemented
   */
  auto file_handle = FileHandleImpFactory::Instance().CreateFileHandleImp(
      kHdfs, partition_id_.getPathAndName());
  if (file_handle == nullptr) return rGetFileHandleFail;
  HdfsInMemoryChunk chunk_his;
  auto begin = old_his_cp;
  auto end = new_his_cp;
  while (begin < end) {
    if (!BlockManager::getInstance()->getMemoryChunkStore()->GetChunk(
            ChunkID(partition_id_, begin / CHUNK_SIZE), chunk_his))
      return rGetMemoryHandleFail;
    auto move = CHUNK_SIZE - (begin + CHUNK_SIZE) % CHUNK_SIZE;
    file_handle->Append(chunk_his.hook, move);
    begin += move;
  }
  return rSuccess;
}

bool PartitionStorage::PersistDisk(UInt64 old_his_cp, UInt64 new_his_cp) {}
