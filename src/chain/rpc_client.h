/*
 * Authors: sm106@illinois.edu
 * This file defines the RPC Client.
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

using chain::ChainImpl;

using namespace std;

class RPCClient {
 private:
    // Target channel for the servers/replicas.
    std::shared_ptr<Channel> channel_;

    // Target stub.
    std::unique_ptr<ChainImpl::Stub> stub_;

 public:
  // Constructor.
  RPCClient(string target_str);

  // Handle client put request.
  void Put(string key, string value, string source_ip);

  // Handle forward request
  bool Forward(string key, string value, string source_ip);

  // Handle the ack request.
  void Ack(string key);

  // Handle the Get request.
  std::string Get(string key);
};

#endif
