#include "chain_client.h"
#include "client_rpc_server.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <mutex>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using chain::ChainImpl;
using chain::PutArg;
using chain::PutRet;
using namespace std;

//-----------------------------------------------------------------------------

ClientRPCServer::ClientRPCServer(ChainClient* chain_client) :
  chain_client_(chain_client) {

}

//-----------------------------------------------------------------------------

// Logic and data behind the server's behavior.
Status ClientRPCServer::Put(ServerContext* context,
                            const chain::PutArg* request,
                            chain::PutRet* reply) {

  cout << "In RPC Server Put" << endl;
  chain_client_->HandleReceiveRequest();
  cout << "return value is " << reply->val();
  return Status::OK;
}

//-----------------------------------------------------------------------------

void ClientRPCServer::RunServer() {
  std::string server_address("0.0.0.0:50054");

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(this);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

//-----------------------------------------------------------------------------