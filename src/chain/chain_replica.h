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
               int replica_id);

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

  // Queue for requests.
  std::queue<std::pair<std::string, std::pair<std::string, std::string>>>
    put_queue_;

  // Mutex for protecting the put queue.
  std::mutex put_mutex_;
};

#endif
