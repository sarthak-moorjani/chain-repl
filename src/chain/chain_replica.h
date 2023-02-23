/*
 * Authors: sm106@illinois.edu
 * This file defines the replica for the Chain replication algorithm.
 *
 */

#ifndef CHAIN_REPLICA_H
#define CHAIN_REPLICA_H

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

class ChainReplica final : public chain::ChainImpl::Service {
  public:
    // The RPC methods definition will come here.
    grpc::Status Put(grpc::ServerContext* context,
                     const chain::PutArg* request,
                     chain::PutRet* reply) override;
};

#endif
