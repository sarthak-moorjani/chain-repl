#include "rpc_server.h"

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

// Logic and data behind the server's behavior.
Status RPCServer::Put(ServerContext* context,
                      const chain::PutArg* request,
                      chain::PutRet* reply) {

  cout << "In Replica Put" << endl;
  reply->set_val("check");
  return Status::OK;
}

//-----------------------------------------------------------------------------

void RunServer() {
  std::string server_address("0.0.0.0:50052");
  RPCServer service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
  //string client_id = std::stoi(argv[1]);
  RunServer();
  return 0;
}
