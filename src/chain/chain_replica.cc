#include "chain_replica.h"
#include "rpc_server.h"
#include "rpc_client.h"

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <fstream>
#include <mutex>
#include <iostream>

using chain::PutArg;
using chain::PutRet;

using namespace std;

//-----------------------------------------------------------------------------

ChainReplica::ChainReplica(vector <int> replica_ids,
                           vector<string> replica_ips,
                           int replica_id) :
  id_(replica_id) {
  rpc_server_ = make_shared<RPCServer>(this);

  // Creating client for each replica in the chain
  for (int i = 0; i < replica_ids.size(); i++) {
    if (replica_ids[i] == id_) {
      continue;
    }
    replica_map_[replica_ids[i]] = make_shared<RPCClient>(replica_ips[i]);
  }

  rpc_server_->RunServer();
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleReplicaPut(const chain::PutArg* request,
                                    chain::PutRet* reply) {
  // Execute the operation in the current replica
  cout << "In replica put" << endl;

  // If current replica is not the tail then forward the request to the next 
  // replica
  if (replica_map_.find(id_ + 1) != replica_map_.end()) {
    ForwardRequest(request->key(), request->val());
    reply->set_val("forwarded");
  } else {
    // If current replica is the tail then respond to the client
    reply->set_val("Hello world");
  }
}

//-----------------------------------------------------------------------------

//Forward request to the next replica or client
void ChainReplica::ForwardRequest(string key, string val) {
  // Send the request to the following replica
  replica_map_[id_ + 1]->Put(key, val);
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

  cout << "Listening to requests " << endl;
}
