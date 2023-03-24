/*
 * Authors: sm106@illinois.edu
 * This file defines the client for the Chain replication service.
 *
 */

#ifndef CHAIN_CLIENT_H
#define CHAIN_CLIENT_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"
#include "rpc_client.h"

// Forward declaration
class ClientRPCServer;

class ChainClient {
 private:

  // ClientRPCServer.
  shared_ptr<ClientRPCServer> client_rpc_server_;

  // Mapping from replica ID to RPC Clients.
  std::unordered_map<int, std::shared_ptr<RPCClient> > replica_map_;

  // Head replica id.
  const int head_replica_id_;

 public:
  // Constructor.
  ChainClient(std::vector<std::string> target_strs =
                std::vector<std::string> (),
              std::vector<int> target_ids,
              int head_id);

  // Run the server.
  void RunServer(std::string ip);

  // Function to perform next operation
  void NextOperation();

  // Handle receive request.
  void HandleReceiveRequest(const chain::AckArg* ack_arg);

  // Create a Put request.
  void Put(std::string key, std::string value, std::string source_ip);

  // Create a Get request.
  void Get(string key);

  // Init the queue with input data
  void InitQueue(string file_path);

  // Make the first operation call.
  void FirstCall();

  // Operations queue.
  std::queue<std::string> operations_queue_;

  // Keys queue
  std::queue<std::string> keys_queue_;

  // Values queue
  std::queue<std::string> values_queue_;

  // Client IP
  std::string client_ip_;

  // Counter for next ops.
  std::atomic<int> next_ops_ctr_;
  // End time of all experiments.
  std::chrono::time_point<std::chrono::high_resolution_clock> end_time_;
};

#endif
