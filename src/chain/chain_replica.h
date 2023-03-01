/*
 * Authors: sm106@illinois.edu
 *
 * This file defines a replica for the Chain Replication service.
 *
 */

#ifndef CHAIN_REPLICA_H
#define CHAIN_REPLICA_H

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

// Forward declaration.
class RPCServer;

class ChainReplica {
 public:
  // Constructor.
  ChainReplica();

  // Handle put request.
  void HandleReplicaPut(const chain::PutArg* request,
                        chain::PutRet* reply);

 private:
  // RPC Server Object.
  std::shared_ptr<RPCServer> rpc_server_;
};

#endif
