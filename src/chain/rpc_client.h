/*
 * Authors: sm106@illinois.edu
 * This file defines the client for the Chain Replication algorithm.
 *
 */

#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::Service;
using grpc::ClientAsyncResponseReader;

using namespace std;

class RPCClient {
 private:
    // Target IPs for the servers/replicas.
    std::map<std::string,std::shared_ptr<Channel>> channels_;

 public:
  // Constructor.
  RPCClient(std::vector<std::string> target_strs =
              std::vector<std::string> ());
};

#endif
