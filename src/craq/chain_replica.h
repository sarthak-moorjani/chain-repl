/*
 * Authors: sm106@illinois.edu
 *
 * This file defines a replica for the Chain Replication service.
 *
 */

#ifndef CHAIN_REPLICA_H
#define CHAIN_REPLICA_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "chain.grpc.pb.h"

// Forward declaration.
class RPCServer;
class RPCClient;

class ChainReplica {
 public:
  // Constructor.
  ChainReplica(std::vector<int> replica_ids,
               std::vector<std::string> replica_ips,
               int replica_id, int tail_id);

  // Run the server.
  void RunServer();

  // Handle put request.
  void HandleReplicaPut(const chain::PutArg* request,
                        chain::PutRet* reply);

  // Forward the request to the next replica in the chain.
  void HandleForwardRequest(const chain::FwdArg* fwd_request,
                            chain::FwdRet* fwd_reply);

  // Handle the background requests.
  void HandlePutQueue();

  // Handle the background forward requests.
  void HandleForwardQueue();

  // Handle the Get request.
  grpc::Status HandleGetRequest(const chain::GetArg* get_arg,
                        chain::GetRet* get_reply);

  // Handle the finalize key request.
  void HandleFinalizeKey(const chain::FinalizeKeyArg* fin_key_arg,
                         chain::FinalizeKeyRet* fin_key_ret);

  // Handle the background finalize requests.
  void HandleFinalizeQueue();

 private:
  // Ack client.
  void AcknowledgeClient(std::string key, std::string source_ip);

 private:
  // RPC Server Object.
  std::shared_ptr<RPCServer> rpc_server_;

  // Mapping from replica ID to RPC Clients.
  std::unordered_map<int, std::shared_ptr<RPCClient> > replica_map_;

  // Maintaining a map of known clients
  std::unordered_map<std::string, std::shared_ptr<RPCClient> > client_map_;

  // Variable to hold this replica's id.
  const int id_;

  // Queue for put requests.
  std::queue<std::pair<std::string, std::pair<std::string, std::string>>>
    put_queue_;

  // Mutex for protecting the put queue.
  std::mutex put_mutex_;

  // Queue for forward requests.
  std::queue<std::pair<std::string, std::pair<std::string, std::string>>>
    forward_queue_;

  // Mutex for protecting forward queue.
  std::mutex forward_mutex_;

  // Map to hold the key value data sent by the user
  std::unordered_map<std::string, std::pair<std::string, bool>> kv_store_;

  // Mutex for protecting the kv store map.
  std::mutex store_mutex_;

  // Queue for finalize requests.
  std::queue<std::string> finalize_queue_;

  // Mutex for protecting the finalize queue.
  std::mutex finalize_mutex_;

  // The tail replica
  const int tail_replica_id_;
};

#endif
