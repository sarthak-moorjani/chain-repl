/*
 * Authors: sm106@illinois.edu
 *
 */

#include "chain_client.h"
#include "client_rpc_server.h"

#include "chain.grpc.pb.h"

#include <assert.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>

using chain::ChainImpl;
using chain::AckArg;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainClient::ChainClient(vector<string> target_strs) :
  rpc_client_(make_shared<RPCClient>(target_strs[0])),
  tail_rpc_client_(make_shared<RPCClient>(target_strs[target_strs.size() - 1])) {

}

//-----------------------------------------------------------------------------

void ChainClient::RunServer(string server_port) {
  client_rpc_server_ = make_shared<ClientRPCServer>(this);
  client_rpc_server_->RunServer(server_port);
}

//-----------------------------------------------------------------------------

void ChainClient::HandleReceiveRequest(const AckArg* ack_arg) {
  cout << "In chain client, received ack for key: "
       << ack_arg->key() << endl;
  if (request_queue_.size() == 0) {
    cout << "nothing more to send" << endl;
    return;
  }
  pair <string, string> p = request_queue_.front();
  request_queue_.pop();
  Put(p.first, p.second, client_ip_);
}

//-----------------------------------------------------------------------------

void ChainClient::Put(string key, string value, string source_ip) {
  rpc_client_->Put(key, value, source_ip);
}

//-----------------------------------------------------------------------------

void ChainClient::Get(string key) {
  string val = tail_rpc_client_->Get(key);
  cout << "Got value " << val << endl;
}

//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  vector<string> server_ips(2);

  ifstream config_file;
  config_file.open(argv[2]);

  string config_line;

  if (config_file.is_open()) {
    while (config_file) {
      std::getline(config_file, config_line);

      if (config_line.size() == 0) {
        break;
      }
       // Get the id of first replica or head
       if (stoi(config_line.substr(0, config_line.find(","))) == 1) {
         server_ips[0] = config_line.substr(config_line.find(",") + 1);
       }
       if (stoi(config_line.substr(0, config_line.find(","))) == 3) {
         server_ips[1] = config_line.substr(config_line.find(",") + 1);
         break;
       }
    } 
  } else {
      cout << "Could not open file";
  }

  ChainClient chain_client(server_ips);

  chain_client.client_ip_ = argv[1];

  string client_port = chain_client.client_ip_.substr
                                   (chain_client.client_ip_.find(":") + 1);

  thread t1(&ChainClient::RunServer, &chain_client, client_port);
  sleep(5);
  for (int i = 0; i < 100; i++) {
    chain_client.request_queue_.push(make_pair("key" + to_string(i), "value"));
    // chain_client.Put("key" + to_string(i), "value", client_ip);
  }
  pair <string, string> p = chain_client.request_queue_.front();
  cout << "first key: " << p.first << endl;
  chain_client.Put(p.first, p.second, chain_client.client_ip_);

  sleep(5);
  for (int i = 0; i < 100; i++) {
    chain_client.Get("key" + to_string(i));
  }

  t1.join();
}
