/*
 * Authors: sm106@illinois.edu
 *
 */

#include "rpc_client.h"

#include "chain.grpc.pb.h"

#include <assert.h>
#include <fstream>
#include <sstream>

using chain::ChainImpl;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using chain::PutArg;
using chain::PutRet;
using chain::AckArg;
using chain::AckRet;

using namespace std;

//-----------------------------------------------------------------------------

RPCClient::RPCClient(string target_str) {

  channel_ = grpc::CreateChannel(target_str,
                                   grpc::InsecureChannelCredentials());
  cout << "Created" << endl;
  stub_ = make_unique<ChainImpl::Stub>(channel_);
}

//-----------------------------------------------------------------------------

void RPCClient::Put(string key, string value) {
  PutArg put_arg;
  put_arg.set_key(key);
  put_arg.set_val(value);

  PutRet put_ret;

  ClientContext context;
  Status status = stub_->Put(&context, put_arg, &put_ret);

  if (status.ok()) {
    std::cout << put_ret.val() << endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

//-----------------------------------------------------------------------------

void RPCClient::Ack(string key) {
  AckArg ack_arg;
  ack_arg.set_key(key);

  AckRet ack_ret;

  ClientContext context;
  Status status = stub_->Ack(&context, ack_arg, &ack_ret);

  if (status.ok()) {
    std::cout << "ok" << endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

//-----------------------------------------------------------------------------
