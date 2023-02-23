/*
 * Authors: sm106@illinois.edu
 *
 */

#include "chain_client.h"

#include "chain.grpc.pb.h"

#include <assert.h>
#include <fstream>
#include <sstream>

using chain::ChainImpl;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainClient::ChainClient(vector<string> target_strs) {
  for (int i =0; i< target_strs.size();i++) {
    std::shared_ptr<Channel> channel =
      grpc::CreateChannel(target_strs[i], grpc::InsecureChannelCredentials());
      cout << "Created" << endl;
      channels_[target_strs[i]] = channel;
  }
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
  ChainClient chain_client({"0.0.0.0:50052"});

  return 0;
}
