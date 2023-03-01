/*
 * Authors: sm106@illinois.edu
 *
 * This file defines a RPC Server.
 *
 */

#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

class RPCServer final : public chain::ChainImpl::Service {
  public:
    // The RPC methods definition will come here.
    grpc::Status Put(grpc::ServerContext* context,
                     const chain::PutArg* request,
                     chain::PutRet* reply) override;
};

#endif
