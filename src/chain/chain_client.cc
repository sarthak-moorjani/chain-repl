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
using chain::AckArg;

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

void ChainClient::HandleReceiveRequest(const AckArg* ack_arg) {
  cout << "In chain client, received ack for key: "
       << ack_arg->key() << endl;
}

//-----------------------------------------------------------------------------

void ChainClient::Put(string key, string value, string source_ip) {
  rpc_client_->Put(key, value, source_ip);
}

//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  vector<string> server_ips;
  server_ips.push_back("0.0.0.0:50052");
  ChainClient chain_client(server_ips);
  
  string client_ip = argv[1];

  thread t1(&ChainClient::RunServer, &chain_client, "0.0.0.0:50054");
  for (int i = 0; i < 10; i++) {
    chain_client.Put("key" + to_string(i), "value", client_ip);
  }
  t1.join();
}
