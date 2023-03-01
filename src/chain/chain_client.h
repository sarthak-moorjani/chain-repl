/*
 * Authors: sm106@illinois.edu
 * This file defines the client for the Chain replication service.
 *
 */

#ifndef CHAIN_CLIENT_H
#define CHAIN_CLIENT_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"
#include "rpc_client.h"

class ChainClient {
 private:
  // RPC Client.
  shared_ptr<RPCClient> rpc_client_;

 public:
  // Constructor.
  ChainClient(std::vector<std::string> target_strs =
                std::vector<std::string> ());

  // Create a Put request.
  void Put(std::string key, std::string value);
};

#endif
