#include "chain_replica.h"
#include "rpc_server.h"
#include "rpc_client.h"

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <fstream>
#include <mutex>
#include <iostream>

using grpc::Status;

using chain::PutArg;
using chain::PutRet;

using namespace std;

//-----------------------------------------------------------------------------

ChainReplica::ChainReplica(vector <int> replica_ids,
                           vector<string> replica_ips,
                           int replica_id) :
  id_(replica_id) {
  rpc_server_ = make_shared<RPCServer>(this);

  num_replicas_ = replica_ids.size();
  for (int i = 0; i < num_replicas_; i++) {
    replica_ids_.push_back(replica_ids[i]);
    replica_ips_.push_back(replica_ips[i]);
  }

  // Creating client for each replica in the chain
  for (int i = 0; i < replica_ids.size(); i++) {
    if (replica_ids[i] == id_) {
      continue;
    }
    replica_map_[replica_ids[i]] = make_shared<RPCClient>(replica_ips[i]);
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleReorganization(int replica_id) {
  int index = 0;
  for (int i = 0; i < replica_ids_.size(); i++) {
    if (replica_ids_[i] == replica_id) {
      index = i;
      break;
    }
  }

  if (id_ > replica_id) {
    id_--;
  }

  num_replicas_--;
  replica_ids_.erase(replica_ids_.begin() + index);
  for (int i = 0; i < num_replicas_; i++) {
    replica_ids_[i] = i + 1;
  }
  replica_map_.clear();
  replica_ips_.erase(replica_ips_.begin() + index);

  // Reinitialize the map.
  for (int i = 0; i < replica_ids_.size(); i++) {
    if (replica_ids_[i] == id_) {
      continue;
    }
    replica_map_[replica_ids_[i]] = make_shared<RPCClient>(replica_ips_[i]);
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::RunServer() {
  rpc_server_->RunServer();
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleReplicaPut(const chain::PutArg* request,
                                    chain::PutRet* reply) {
  // Execute the operation in the current replica
  //cout << "In replica put" << endl;
  // Add data to the kv store.
  {
    std::unique_lock<std::mutex> lock(store_mutex_);
    kv_store_[request->key()] = request->val();
  }

  {
    std::unique_lock<std::mutex> lock(put_mutex_);
    put_queue_.push(make_pair(request->key(),
                              make_pair(request->val(), request->source_ip())));
    reply->set_val("forwarded");
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::HandlePutQueue() {

  while (true) {
    bool send_req = false;
    pair<string, pair<string, string> > p;
    if (!put_queue_.empty()) {
      {
        std::unique_lock<std::mutex> lock(put_mutex_);
        p = put_queue_.front();
        send_req = true;
        put_queue_.pop();
      }
    }
    if (send_req) {
      bool send_ok =
        replica_map_[id_ + 1]->Forward(p.first, p.second.first, p.second.second);
      if (!send_ok) {
        HandleReorganization(id_ + 1);
        while (!send_ok) {
          cout << "resending req" << endl;
          send_ok =
            replica_map_[id_ + 1]->Forward(p.first, p.second.first, p.second.second);
        }
      }
    }
  }
}

Status ChainReplica::HandleGetRequest(const chain::GetArg* get_arg,
                                    chain::GetRet* get_reply) {

  if (kv_store_.find(get_arg->key()) == kv_store_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Key not found");
  }

  get_reply->set_value(kv_store_[get_arg->key()]);
  return Status::OK;
}

//-----------------------------------------------------------------------------

//Forward request to the next replica or client
void ChainReplica::HandleForwardRequest(const chain::FwdArg* request, 
		                        chain::FwdRet* reply) {
	// Add data in the kv store.
  {
    std::unique_lock<std::mutex> lock(store_mutex_);
    kv_store_[request->key()] = request->val();
  }
	// Add the request to the forward queue.
  {
    std::unique_lock<std::mutex> lock(forward_mutex_);
    forward_queue_.push(make_pair(request->key(),
                              make_pair(request->val(), request->source_ip())));
    // If current replica is not the tail then forwward the request to the next
    // replica.
    if (replica_map_.find(id_ + 1) != replica_map_.end()) {
        reply->set_val("forwarded");
    } else {
        // If current replica is the tail then respond to the client
        //cout << "This is the tail, sent ack to client" << endl;
        reply->set_val("sent-to-client");
    }
  }
}

//-----------------------------------------------------------------------------

// Handle requests in the forward queue.
void ChainReplica::HandleForwardQueue() {
  //cout << "In handle forward queue" << endl;
  while (true) {
    bool forward_req = false;
    pair<string, pair<string, string> > p;
    if (!forward_queue_.empty()) {
      {
        std::unique_lock<std::mutex> lock(forward_mutex_);
        p = forward_queue_.front();
        forward_req = true;
        forward_queue_.pop();
      }
    }
    if (forward_req) {
      // If current replica is not the tail then forward the request to the
      // next replica
      if (replica_map_.find(id_ + 1) != replica_map_.end()) {
        bool send_ok = replica_map_[id_ + 1]->Forward(p.first, p.second.first,
                                                      p.second.second);
        if (!send_ok) {
          HandleReorganization(id_ + 1);
          while (!send_ok) {
            if (id_ == num_replicas_) {
              cout << "Tail failed, I am the current tail, acking client"
                   << endl;
              AcknowledgeClient(p.first, p.second.second);
              send_ok = true;
            } else {
              cout << "resending req" << endl;
              send_ok =
                replica_map_[id_ + 1]->Forward(p.first, p.second.first,
                                               p.second.second);
            }
          }
        }
      } else {
        // If current replica is the tail then respond to the client
        //cout << "trying to send an ack to the client" << endl;
        AcknowledgeClient(p.first, p.second.second);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::AcknowledgeClient(string key, string source_ip) {
  // Checking if the client is already known, then simply send the ack.
//  cout << "sending ack to " << source_ip << endl;
  if (client_map_.find(source_ip) != client_map_.end()) {
    client_map_[source_ip]->Ack(key);
  } else {
  // Add the new client to the map and then send the ack to client.
    std::shared_ptr<RPCClient> rpc_client = make_shared<RPCClient>(source_ip);
    client_map_[source_ip] = rpc_client;
    rpc_client->Ack(key);
  }
}

//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  // Vectors to store information of replicas in the chain
  std::vector<int> replica_ids;
  std::vector<string> replica_ips;

  // Creating stream to read config file
  std::ifstream config_file;

  // Opening the file
  config_file.open(argv[2]);

  // String to read each line from file.
  std::string config_line;

  // Reading from the file.
  if (config_file.is_open()) {
	  while (config_file) {
		  std::getline(config_file, config_line);

      if (config_line.size() == 0) {
        break;
      }
		  // Get the id of other replicas
		  replica_ids.push_back(stoi(config_line.substr(0, config_line.find(","))));
		  replica_ips.push_back(config_line.substr(config_line.find(",") + 1));
    }
	} else {
    cout << "Could not open file";
  }

  for (int i = 0; i < replica_ids.size(); i++) {
    cout << replica_ids[i] << " " << replica_ips[i] << endl;
  }

  cout << argv[1] << endl;
  int replica_id = std::stoi(argv[1]);
  cout << "Current replica id is " << replica_id << endl;
  ChainReplica chain_replica(replica_ids, replica_ips, replica_id);
  thread t1(&ChainReplica::HandlePutQueue, &chain_replica);
  thread t2(&ChainReplica::HandleForwardQueue, &chain_replica);
  thread t3(&ChainReplica::RunServer, &chain_replica);

  t1.join();
  t2.join();
  t3.join();
  cout << "Listening to requests " << endl;
}
