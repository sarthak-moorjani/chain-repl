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
 typedef std::chrono::time_point<std::chrono::high_resolution_clock> nowtime;

 private:
  // ClientRPCServer.
  shared_ptr<ClientRPCServer> client_rpc_server_;

  // Mapping from replica ID to RPC Clients.
  std::unordered_map<int, std::shared_ptr<RPCClient> > replica_map_;

  // Head replica id.
  const int replica_count_;

 public:

  int key_counter_;
 public:
  // Constructor.
  ChainClient(std::vector<std::string> target_strs,
              std::vector<int> target_ids);

  // Run the server.
  void RunServer(std::string ip);

  // Function to perform next operation
  void NextOperation();

  // Handle receive request.
  void HandleReceiveRequest(const chain::AckArg* ack_arg);

  // Create a Put request.
  void Put(std::string key, std::string value, std::string source_ip);

  // Create a Get request.
  void Get(string key, int replica_id);

  // Init the queue with input data
  void InitQueue(string file_path);

  // Make the first operation call.
  void FirstCall();

  // Method to test the code changes
  void TestMethod(std::string op);

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
  nowtime end_time_;

  // Put Latency times recorder.
  map<std::string, pair<nowtime, nowtime>> put_latency_tracker_;

  // Get Latency times recorder.
  map<std::string, pair<nowtime, nowtime>> get_latency_tracker_;
};

#endif
