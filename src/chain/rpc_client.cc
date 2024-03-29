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
using chain::FwdArg;
using chain::FwdRet;
using chain::GetArg;
using chain::GetRet;

using namespace std;

//-----------------------------------------------------------------------------

RPCClient::RPCClient(string target_str) {

  channel_ = grpc::CreateChannel(target_str,
                                   grpc::InsecureChannelCredentials());
  cout << "Created" << endl;
  stub_ = make_unique<ChainImpl::Stub>(channel_);
}

//-----------------------------------------------------------------------------

void RPCClient::Put(string key, string value, string source_ip) {
  PutArg put_arg;
  put_arg.set_key(key);
  put_arg.set_val(value);
  put_arg.set_source_ip(source_ip);

  PutRet put_ret;

  ClientContext context;
  Status status = stub_->Put(&context, put_arg, &put_ret);

  if (status.ok()) {
    //std::cout << "put rpc completed" << endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

//-----------------------------------------------------------------------------

string RPCClient::Get(string key) {
  GetArg get_arg;
  get_arg.set_key(key);

  GetRet get_ret;

  ClientContext context;
  Status status = stub_->Get(&context, get_arg, &get_ret);

  if (status.ok()) {
    //std::cout << "get rpc completed" << endl;
    return get_ret.value();
  } else {
    //std::cout << status.error_code() << ": " << status.error_message()
    //          << std::endl;
    return "";
  }
}

//-----------------------------------------------------------------------------

bool RPCClient::Forward(string key, string value, string source_ip) {
  FwdArg fwd_arg;
  fwd_arg.set_key(key);
  fwd_arg.set_val(value);
  fwd_arg.set_source_ip(source_ip);

  FwdRet fwd_ret;

  ClientContext context;
  Status status = stub_->Forward(&context, fwd_arg, &fwd_ret);

  if (status.ok()) {
    return true;
   // std::cout << "forward rpc completed" << endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return false;
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
  //  std::cout << "ack rpc completed" << endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

//-----------------------------------------------------------------------------
