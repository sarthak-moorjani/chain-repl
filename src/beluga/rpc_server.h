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

// Forward declaration.
class ChainReplica;

class RPCServer final : public chain::ChainImpl::Service {
  public:
    // Constructor.
    RPCServer(ChainReplica *chain_replica);

    // Run the server.
    void RunServer();

    // The RPC methods definition will come here.
    grpc::Status Put(grpc::ServerContext* context,
                     const chain::PutArg* request,
                     chain::PutRet* reply) override;

    // RPC method to handle forward request from previous replica.
    grpc::Status Forward(grpc::ServerContext* context,
		                     const chain::FwdArg* fwd_request,
			                   chain::FwdRet* fwd_reply) override;

    // RPC method to handle read requests from the tail.
    grpc::Status Get(grpc::ServerContext* context,
                     const chain::GetArg* get_request,
                     chain::GetRet* get_reply) override;

    // RPC method to handle the back propagation for a key.
    grpc::Status FinalizeKey(grpc::ServerContext* context,
                             const chain::FinalizeKeyArg* fin_key_arg,
                             chain::FinalizeKeyRet* fin_key_ret);

 private:
  // Chain replica object.
  ChainReplica* chain_replica_;
};

#endif
