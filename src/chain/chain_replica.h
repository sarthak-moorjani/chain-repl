/*
 * Authors: sm106@illinois.edu
 *
 * This file defines a replica for the Chain Replication service.
 *
 */

#ifndef CHAIN_REPLICA_H
#define CHAIN_REPLICA_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

// Forward declaration.
class RPCServer;
class RPCClient;

class ChainReplica {
 public:
  // Constructor.
  ChainReplica(std::vector<int> replica_ids,
               std::vector<std::string> replica_ips,
               int replica_id);

  // Handle put request.
  void HandleReplicaPut(const chain::PutArg* request,
                        chain::PutRet* reply);

 private:
  // Forward the request to the next replica in the chain.
  void ForwardRequest(std::string key, std::string val);

 private:
  // RPC Server Object.
  std::shared_ptr<RPCServer> rpc_server_;

  // Mapping from replica ID to RPC Clients.
  std::unordered_map<int, std::shared_ptr<RPCClient> > replica_map_;

  // Variable to hold this replica's id.
  const int id_;
};

#endif
