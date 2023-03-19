#include "chain_replica.h"
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
using chain::FwdArg;
using chain::FwdRet;
using namespace std;

//-----------------------------------------------------------------------------

RPCServer::RPCServer(ChainReplica* chain_replica) :
  chain_replica_(chain_replica) {

}

//-----------------------------------------------------------------------------

// Logic and data behind the server's behavior.
Status RPCServer::Put(ServerContext* context,
                      const chain::PutArg* request,
                      chain::PutRet* reply) {

  cout << "In RPC Server Put" << endl;
  chain_replica_->HandleReplicaPut(request, reply);
  return Status::OK;
}

//-----------------------------------------------------------------------------

Status RPCServer::Forward(ServerContext* context,
		                      const chain::FwdArg* fwd_request,
			                    chain::FwdRet* fwd_reply) {
  cout << "In RPC Server Forward" << endl;
  chain_replica_->HandleForwardRequest(fwd_request, fwd_reply);
  return Status::OK;
}

//-----------------------------------------------------------------------------

Status RPCServer::Get(ServerContext* context,
                      const chain::GetArg* get_request,
                      chain::GetRet* get_reply) {
  cout << "In RPC Server Get" << endl;
  return chain_replica_->HandleGetRequest(get_request, get_reply);
  //return Status::OK;
}

//-----------------------------------------------------------------------------

void RPCServer::RunServer() {
  std::string server_address("0.0.0.0:50052");

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
