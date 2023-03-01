#include "chain_replica.h"

#include "chain.grpc.pb.h"
#include <bits/stdc++.h>
#include <mutex>

using chain::PutArg;
using chain::PutRet;
using namespace std;

//-----------------------------------------------------------------------------

void ChainReplica::HandleReplicaPut(const chain::PutArg* request,
                                    chain::PutRet* reply) {

  reply->set_val("Hello world");
}

//-----------------------------------------------------------------------------
