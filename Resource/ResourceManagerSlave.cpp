/*
 * ResourceManagerSlave.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: wangli
 */

#include "ResourceManagerSlave.h"

#include <glog/logging.h>
#include <string>
#include <exception>

#include "../Environment.h"
#include "../node_manager/base_node.h"
#include "caf/io/all.hpp"
#include "caf/all.hpp"
#include "../Config.h"
#include "../loader/load_packet.h"

using caf::io::remote_actor;
using claims::loader::RegNodeAtom;
using caf::after;
using claims::NodeAddr;
using claims::OkAtom;
using claims::StorageBudgetAtom;
InstanceResourceManager::InstanceResourceManager() {}

InstanceResourceManager::~InstanceResourceManager() {}

void InstanceResourceManager::ReportStorageBudget(
    StorageBudgetMessage& message) {
  caf::scoped_actor self;
  auto master_actor =
      Environment::getInstance()->get_slave_node()->GetMasterActor();
  self->sync_send(master_actor, StorageBudgetAtom::value, message).await(

      [=](OkAtom) { LOG(INFO) << "reporting storage budget is ok!" << endl; },
      after(std::chrono::seconds(30)) >>
          [=]() {
            LOG(WARNING) << "reporting storage budget, but timeout 30s !!"
                         << endl;
          });
     LOG(INFO)<<"node :"<<message.nodeid<<"report storage finish"<<endl;
}

void InstanceResourceManager::setStorageBudget(unsigned long memory,
                                               unsigned long disk) {}

// TODO(Yukai): insert the below code into current version
NodeID InstanceResourceManager::Register() {
  //  NodeID ret = 10;
  //  TimeOutReceiver receiver(Environment::getInstance()->getEndPoint());
  //  Theron::Catcher<NodeID> resultCatcher;
  //  receiver.RegisterHandler(&resultCatcher, &Theron::Catcher<NodeID>::Push);
  //
  //  std::string ip = Environment::getInstance()->getIp();
  //  unsigned port = Environment::getInstance()->getPort();
  //  NodeRegisterMessage message(ip, port);
  //
  //  DLOG(INFO) << "resourceManagerSlave is going to register (" << ip << ","
  //             << port << ")to master";
  //  framework_->Send(message, receiver.GetAddress(),
  //                   Theron::Address("ResourceManagerMaster"));
  //  Theron::Address from;
  //  if (receiver.TimeOutWait(1, 1000) == 1) {
  //    resultCatcher.Pop(ret, from);
  //    logging_->log(
  //        "Successfully registered to the master, the allocated id =%d.",
  //        ret);
  //
  //    // send all node info to master loader
  //    DLOG(INFO) << "going to send node info to (" << Config::master_loader_ip
  //               << ":" << Config::master_loader_port << ")";
  NodeID ret = 10;
  int retry_max_time = 10;
  int time = 0;
  while (1) {
    try {
      caf::actor master_actor =
          remote_actor(Config::master_loader_ip, Config::master_loader_port);
      caf::scoped_actor self;
      self->sync_send(
                master_actor, RegNodeAtom::value,
                NodeAddress(Environment::getInstance()->getIp(),
                            to_string(Environment::getInstance()->getPort())),
                ret).await([&](int r) {
        LOG(INFO) << "sent node info and received response";
      });
      break;
    } catch (exception& e) {
      cout << "new remote actor " << Config::master_loader_ip << ","
           << Config::master_loader_port << "failed for " << ++time << " time. "
           << e.what() << endl;
      usleep(100 * 1000);
      if (time >= retry_max_time) return false;
    }
  }

  return ret;
  //  } else {
  //    logging_->elog("Failed to get NodeId from the master.");
  //    cerr << "Failed to get NodeId from the master." << endl;
  //    return -1;
  //  }
}
