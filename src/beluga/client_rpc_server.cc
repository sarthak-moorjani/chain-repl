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
using chain::AckArg;
using chain::AckRet;

using namespace std;

//-----------------------------------------------------------------------------

ClientRPCServer::ClientRPCServer(ChainClient* chain_client) :
  chain_client_(chain_client) {

}

//-----------------------------------------------------------------------------

// Logic and data behind the server's behavior.
Status ClientRPCServer::Ack(ServerContext* context,
                            const chain::AckArg* request,
                            chain::AckRet* reply) {

  chain_client_->HandleReceiveRequest(request);
  return Status::OK;
}

//-----------------------------------------------------------------------------

void ClientRPCServer::RunServer(string server_port) {
  std::string server_address("0.0.0.0:" + server_port);

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
