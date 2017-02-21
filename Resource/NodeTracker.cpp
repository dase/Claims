/*
 * NodeTracker.cpp
 *
 *  Created on: Nov 3, 2013
 *      Author: wangli
 */

#include "NodeTracker.h"

#include <assert.h>
#include <glog/logging.h>
#include <vector>
#include "../Environment.h"
NodeTracker* NodeTracker::instance_ = 0;
NodeTracker::NodeTracker() : allocate_cur_(0) {}
NodeTracker* NodeTracker::GetInstance() {
  if (instance_ == 0) {
    instance_ = new NodeTracker();
  }
  return instance_;
}
NodeTracker::~NodeTracker() {
  // TODO Auto-generated destructor stub
}

int NodeTracker::RegisterNode(NodeAddress new_node_address) {
  assert(false);
  if (address_to_id_.find(new_node_address) != address_to_id_.end()) {
    /*node_name already exists.*/
    return -1;
  }
  const int allocated_id = allocate_cur_++;
  address_to_id_[new_node_address] = allocated_id;
  return allocated_id;
}

std::string NodeTracker::GetNodeIP(const NodeID& target) const {
#ifdef THERON
  boost::unordered_map<NodeAddress, NodeID>::const_iterator it =
      address_to_id_.cbegin();
  while (it != address_to_id_.cend()) {
    if (it->second == target) return it->first.ip;
    it++;
  }
  return NULL;  // TODO avoid return NULL in case of no matching target by
                // changing the return type to be boolean.*/
                //	return NULL;
#else
  return Environment::getInstance()
      ->get_master_node()
      ->GetNodeAddrFromId(target)
      .first;
#endif
}

std::vector<NodeID> NodeTracker::GetNodeIDList() const {
  std::vector<NodeID> ret;
#ifdef THERON
  boost::unordered_map<NodeAddress, NodeID>::const_iterator it =
      address_to_id_.cbegin();
  while (it != address_to_id_.cend()) {
    ret.push_back(it->second);
    it++;
  }
#else
  ret = Environment::getInstance()->get_slave_node()->GetAllNodeID();
#endif
  return ret;
}

RetCode NodeTracker::GetNodeAddr(const NodeID& target,
                                 NodeAddress& node_addr) const {
  boost::unordered_map<NodeAddress, NodeID>::const_iterator it =
      address_to_id_.cbegin();
  while (it != address_to_id_.cend()) {
    if (it->second == target) {
      node_addr = it->first;
      return claims::common::rSuccess;
    }
    it++;
  }
  assert(false && "can't find node address according node ID");
  return claims::common::rFailure;
}

RetCode NodeTracker::InsertRegisteredNode(const NodeID& node_id,
                                          const NodeAddress& node_addr) {
  address_to_id_[node_addr] = node_id;
  LOG(INFO) << "inserted node:" << node_id
            << ". Now size of addr2id is:" << address_to_id_.size();
  return claims::common::rSuccess;
}
