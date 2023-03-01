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

using chain::ChainImpl;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainClient::ChainClient(vector<string> target_strs) :
  rpc_client_(make_shared<RPCClient>(target_strs[0])) {

}

//-----------------------------------------------------------------------------

void ChainClient::RunServer(string server_ip) {
  client_rpc_server_ = make_shared<ClientRPCServer>(this);
  client_rpc_server_->RunServer();
}

//-----------------------------------------------------------------------------

void ChainClient::HandleReceiveRequest() {}

//-----------------------------------------------------------------------------

void ChainClient::Put(string key, string value) {
  rpc_client_->Put(key, value);
}

//-----------------------------------------------------------------------------

int main() {
  vector<string> server_ips;
  server_ips.push_back("0.0.0.0:50052");
  ChainClient chain_client(server_ips);

  thread t1(&ChainClient::RunServer, &chain_client, "0.0.0.0:50052");
  for (int i = 0; i < 10; i++) {
    chain_client.Put("key" + to_string(i), "value");
  }
  t1.join();
}
