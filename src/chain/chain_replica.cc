#include "chain_replica.h"
#include "rpc_server.h"

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <mutex>

using chain::PutArg;
using chain::PutRet;
using namespace std;

//-----------------------------------------------------------------------------

ChainReplica::ChainReplica() {
  rpc_server_ = make_shared<RPCServer>(this);
  rpc_server_->RunServer();
}

//-----------------------------------------------------------------------------

void ChainReplica::HandleReplicaPut(const chain::PutArg* request,
                                    chain::PutRet* reply) {

  cout << "In replica put " << endl;
  reply->set_val("Hello world");
}

//-----------------------------------------------------------------------------

int main() {
  ChainReplica chain_replica;
  cout << "Listening to requests " << endl;
}
