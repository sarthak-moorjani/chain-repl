/*
 * Authors: sm106@illinois.edu
 *
 * This file defines the client RPC Server.
 *
 */

#ifndef CLIENT_RPC_SERVER_H
#define CLIENT_RPC_SERVER_H

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

// Forward declaration.
class ChainClient;

class ClientRPCServer final : public chain::ChainImpl::Service {
  public:
    // Constructor.
    ClientRPCServer(ChainClient *chain_client);

    // Run the server.
    void RunServer();

    // The RPC methods definition will come here.
    grpc::Status Put(grpc::ServerContext* context,
                     const chain::PutArg* request,
                     chain::PutRet* reply) override;

 private:
  // Chain client object.
  ChainClient* chain_client_;
};

#endif
