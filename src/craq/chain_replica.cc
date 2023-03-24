#include "chain_replica.h"
#include "rpc_server.h"
#include "rpc_client.h"

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <fstream>
#include <mutex>
#include <iostream>

using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainReplica::ChainReplica(vector <int> replica_ids,
                           vector<string> replica_ips,
                           int replica_id, int tail_id) :
  id_(replica_id),
  tail_replica_id_(tail_id) {
  rpc_server_ = make_shared<RPCServer>(this);

  // Creating client for each replica in the chain
  for (int i = 0; i < replica_ids.size(); i++) {
    if (replica_ids[i] == id_) {
      continue;
    }
    replica_map_[replica_ids[i]] = make_shared<RPCClient>(replica_ips[i]);
  }

  // rpc_server_->RunServer();
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
    kv_store_[request->key()] = make_pair(request->val(),
                                          false /* is_final */);
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
    if (send_req)
      replica_map_[id_ + 1]->Forward(p.first, p.second.first, p.second.second);
  }
}

//-----------------------------------------------------------------------------

Status ChainReplica::HandleGetRequest(const chain::GetArg* get_arg,
                                    chain::GetRet* get_reply) {

  if (kv_store_.find(get_arg->key()) == kv_store_.end()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Key not found");
  }

  if (kv_store_[get_arg->key()].second == true) {
    get_reply->set_value(kv_store_[get_arg->key()].first);
  } else {
    cout << "asking key from tail" << endl;
    // Get from tail.
    string tail_val = replica_map_[tail_replica_id_]->Get(get_arg->key());
    get_reply->set_value(tail_val + " from tail");
  }

  return Status::OK;
}

//-----------------------------------------------------------------------------

//Forward request to the next replica or client
void ChainReplica::HandleForwardRequest(const chain::FwdArg* request, 
		                        chain::FwdRet* reply) {
	// Add data in the kv store.
	// If current replica is the tail then the entry will always be clean.
	{
      std::unique_lock<std::mutex> lock(store_mutex_);
      if (id_ == tail_replica_id_) {
        kv_store_[request->key()] = make_pair(request->val(), true /* is_final */);
      } else {
        kv_store_[request->key()] = make_pair(request->val(), false /* is_final */);
      }
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
      if (id_ != tail_replica_id_) {
        replica_map_[id_ + 1]->Forward(p.first, p.second.first,
                                       p.second.second);
      } else {
        // If current replica is the tail then respond to the client
        //cout << "trying to send an ack to the client" << endl;
        AcknowledgeClient(p.first, p.second.second);
        // Add the request to the finalize queue.
        {
          cout << "this is the tail, adding to finalize queue" << endl;
          std::unique_lock<std::mutex> lock(finalize_mutex_);
          finalize_queue_.push(p.first);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleFinalizeKey(const chain::FinalizeKeyArg* fin_key_arg,
                                     chain::FinalizeKeyRet* fin_key_ret) {

  {
    std::unique_lock<std::mutex> lock(store_mutex_);
    string val = kv_store_[fin_key_arg->key()].first;
    cout << "marking key " << fin_key_arg->key() << " final" << endl;
    kv_store_[fin_key_arg->key()] = make_pair(val, true /* is_final */);
  }

    // Add the request to the finalize queue.
  {
    std::unique_lock<std::mutex> lock(finalize_mutex_);
    finalize_queue_.push(fin_key_arg->key());
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleFinalizeQueue() {
  while (true) {
    bool fin_request = false;
    string key;
    if (!finalize_queue_.empty()) {
      {
        std::unique_lock<std::mutex> lock(finalize_mutex_);
        key = finalize_queue_.front();
        fin_request = true;
        finalize_queue_.pop();
      }
    }
    if (fin_request) {
      if (replica_map_.find(id_ - 1) != replica_map_.end()) {
        cout << "Sending to parent to mark final" << endl;
        replica_map_[id_ - 1]->FinalizeKey(key);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void ChainReplica::AcknowledgeClient(string key, string source_ip) {
  // Checking if the client is already known, then simply send the ack.
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

  int max_id = 0;
  // Reading from the file.
  if (config_file.is_open()) {
	  while (config_file) {
	    std::getline(config_file, config_line);

        if (config_line.size() == 0) {
          break;
        }
		  // Get the id of other replicas
	    int input_id = stoi((config_line.substr(0, config_line.find(","))));
		if (input_id > max_id) {
		  max_id = input_id;
		}
		replica_ids.push_back(input_id);
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
  cout << "Tail replica id " << max_id << endl;
  ChainReplica chain_replica(replica_ids, replica_ips, replica_id, max_id);
  thread t1(&ChainReplica::HandlePutQueue, &chain_replica);
  thread t2(&ChainReplica::HandleForwardQueue, &chain_replica);
  thread t3(&ChainReplica::HandleFinalizeQueue, &chain_replica);
  thread t4(&ChainReplica::RunServer, &chain_replica);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  cout << "Listening to requests " << endl;
}
