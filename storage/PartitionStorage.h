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
 * /Claims/storage/PartitionReaderIterator.cpp
 *
 *  Created on: Nov 14, 2013
 *  Modified on: Nov 28, 2015
 *      Author: wangli, Hanzhang
 *       Email:
 *
 * Description:
 *
 */

#ifndef PARTITIONSTORAGE_H_
#define PARTITIONSTORAGE_H_
#include <atomic>
#include <vector>
#include <unordered_map>
#include "../common/error_define.h"
#include "../txn_manager/txn.hpp"
#include "../storage/ChunkStorage.h"
#include "../storage/StorageLevel.h"
#include "../storage/PartitionReaderIterator.h"
#include "../utility/lock.h"
#include "../utility/lock_guard.h"
#include "../Debug.h"
using claims::txn::PStrip;
using claims::txn::UInt64;
using claims::utility::LockGuard;
// namespace claims {
// namespace storage {
/**
 * Design by The Iterator pattern. It is a container of partition. We should
 * create iterator in the container and use the iterator to operator this
 * container. By operating the partition iterator, it can generate chunk.
 */
class PartitionStorage {
  friend class PartitionReaderIterator;

 public:
  class PartitionReaderIterator {
   public:
    /**
     * @brief Method description: construct the partition iterator.
     * @param PartitionStorage: include the point of container, the cursor of
     * chunk, and chunk_list_.
     */
    PartitionReaderIterator(PartitionStorage* partition_storage);

    virtual ~PartitionReaderIterator();

    /**
     * @brief Method description: According the iterator to call chunk list and
     * create chunk iterator.
     * @return ret: NULL: create the chunk iterator failed. NOt NULL: succeed.
     */
    virtual ChunkReaderIterator* NextChunk();

    virtual bool NextBlock(BlockStreamBase*& block);

   protected:
    PartitionStorage* ps_;
    unsigned chunk_cur_;
    ChunkReaderIterator* chunk_it_;
  };

  class AtomicPartitionReaderIterator : public PartitionReaderIterator {
   public:
    /**
     * @brief Method description: Construct the partition iterator. Different
     * from
     * PartitionReaderiterator, it don't copy next block one by one, just using
     * the block_accessor that store the point of block to assign.
     * @param  PartitionStorage: include the point of container, the cursor of
     * chunk, and chunk_list_.
     */
    AtomicPartitionReaderIterator(PartitionStorage* partition_storage)
        : PartitionReaderIterator(partition_storage) {}

    virtual ~AtomicPartitionReaderIterator() override;

    /**
     * @brief Method description: According the iterator to call chunk list and
     * create chunk iterator. Different from PartitionReaderiterator, it don't
     * copy next block one by one, just using the block_accessor that store the
     * point of block to assign.
     * @return ret: NULL: create the chunk iterator failed. NOt NULL: succeed.
     */
    ChunkReaderIterator* NextChunk() override;

    virtual bool NextBlock(BlockStreamBase*& block);

   private:
    Lock lock_;
  };
  /**********************************************************************/
  class TxnPartitionReaderIterator : public PartitionReaderIterator {
   public:
    /**
     * @brief Method description: Construct the Iterator. Different from
     * PartitionReaderIterator and AtomicPartitionReaditerator.
     * It support scan from  both chunk_list_ and rt_chunk_list_ ,
     * write into rt_chunk_list rather than chunk_list_ in
     * AtomicPartitionReaderIterator
     */

    TxnPartitionReaderIterator(PartitionStorage* partition_storage,
                               uint64_t his_cp,
                               const vector<PStrip>& rt_strip_list);
    ~TxnPartitionReaderIterator() override;
    bool NextBlock(BlockStreamBase*& block) override;

   private:
    void* CreateEmptyBlock() {
      void* data = reinterpret_cast<void*>(malloc(BLOCK_SIZE));
      // auto block = new BlockStreamFix(BLOCK_SIZE, 0, data, 0);
      return data;
    }
    void SetBlockTail(void* block, unsigned tuple_num) {
      *reinterpret_cast<unsigned*>(block + 64 * 1024 - sizeof(unsigned)) =
          tuple_num;
    }
    int64_t last_his_block_;
    int64_t block_cur_;
    int64_t chunk_cur_;

    vector<PStrip> rt_strip_list_;  // splited by block
    int64_t rt_block_index_;
    int64_t rt_block_cur_;
    int64_t rt_chunk_cur_;
    ChunkReaderIterator* rt_chunk_it_;
    vector<void*> rt_block_buffer_;

    Lock lock_;

   public:
  };

  /**
   * @brief Method description: construct the partition container.
   * @param :PartitionID: identify which partition is our require.
   * @param :number_of_chunks: get how many chunks in the partition.
   * @param :storage_level: the information of chunk in which storage level.
   */
  PartitionStorage(const PartitionID& partition_id,
                   const unsigned& number_of_chunks, const StorageLevel&);

  /**
   * @brief Method description: Destruction. Release all chunk_list_.
   */
  virtual ~PartitionStorage();

  void AddNewRTChunk();
  void AddNewHisChunk();

  RetCode AddHisChunkWithMemoryApply(unsigned expected_number_of_chunks,
                                     const StorageLevel& storage_level);

  RetCode AddRtChunkWithMemoryApply(unsigned expected_number_of_chunks,
                                    const StorageLevel& storage_level);

  int GetRTChunkNum() {
    LockGuard<Lock> guard(write_lock_);
    return rt_chunk_list_.size();
  }

  int GetHisChunkNum() {
    LockGuard<Lock> guard(write_lock_);
    return chunk_list_.size();
  }

  /**
   * @brief Method description: Expand the container of partition
   * @param :PartitionID: identify which partition is our require.
   * @param :number_of_chunks: get how many chunks in the partition.
   * @param :storage_level: the information of chunk in which storage level.
   */
  void UpdateChunksWithInsertOrAppend(const PartitionID& partition_id,
                                      const unsigned& number_of_chunks,
                                      const StorageLevel& storage_level);

  /**
   * @brief Method description: Clear the container of partition
   * @param :PartitionID: Choose the partition what we need to delete.
   */
  void RemoveAllChunks(const PartitionID& partition_id);

  /**
  * @brief Method description: Generate the iterator in iterator pattern.
  */
  PartitionStorage::PartitionReaderIterator* CreateReaderIterator();

  /**
   * @brief Method description: Generate the iterator in iterator
   * pattern. Different from PartitionReaderiterator, it don't copy next block
   * one by one, just using the block_accessor that store the point of block.
   */
  PartitionStorage::PartitionReaderIterator* CreateAtomicReaderIterator();

  PartitionStorage::PartitionReaderIterator* CreateTxnReaderIterator(
      uint64_t his_cp, const vector<PStrip>& rt_strip_list) {
    return new TxnPartitionReaderIterator(this, his_cp, rt_strip_list);
  }
  void CheckAndAppendChunkList(unsigned number_of_chunk, bool is_rt);
  UInt64 MergeToHis(UInt64 old_his_cp, const vector<PStrip>& strip_list);
  RetCode Persist(UInt64 old_his_cp, UInt64 new_his_cp);
  bool PersistHDFS(UInt64 old_his_cp, UInt64 new_his_cp);
  bool PersistDisk(UInt64 old_his_cp, UInt64 new_his_cp);

 protected:
  PartitionID partition_id_;
  atomic<unsigned> number_of_chunks_;
  std::vector<ChunkStorage*> chunk_list_;
  // add it for txn scan
  atomic<unsigned> number_of_rt_chunks_;
  std::vector<ChunkStorage*> rt_chunk_list_;
  StorageLevel desirable_storage_level_;

  Lock write_lock_;

  // static ofstream logfile;
};
//}  // namespace storage
//}  // namespace claims

#endif /* PARTITIONSTORAGE_H_ */
