/*
 * NodeTracker.h
 *
 *  Created on: Nov 3, 2013
 *      Author: wangli
 *
 *  NodeTrakcer allocates a unique Id for each node.
 */

#ifndef NODETRACKER_H_
#define NODETRACKER_H_
#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "../common/error_define.h"
#include "../common/ids.h"
#ifdef DMALLOC
#include "./dmalloc.h"
#endif
typedef std::string NodeIP;  // TODO: may use ip + port to support multiple
                             // instances on a single node.

typedef int NodeID;
class NodeTracker {
 public:
  static NodeTracker* GetInstance();
  virtual ~NodeTracker();
  int RegisterNode(NodeAddress);
  std::string GetNodeIP(const NodeID&) const;
  RetCode GetNodeAddr(const NodeID&, NodeAddress& node_addr) const;

  RetCode InsertRegisteredNode(const NodeID& node_id,
                               const NodeAddress& node_addr);

  std::vector<NodeID> GetNodeIDList() const;

 private:
  NodeTracker();
  boost::unordered_map<NodeAddress, NodeID> address_to_id_;
  unsigned allocate_cur_;
  static NodeTracker* instance_;
};

#endif /* NODETRACKER_H_ */
