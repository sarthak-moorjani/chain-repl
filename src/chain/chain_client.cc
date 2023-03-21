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
#include <unistd.h>

using chain::ChainImpl;
using chain::AckArg;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainClient::ChainClient(vector<string> target_strs) :
  rpc_client_(make_shared<RPCClient>(target_strs[0])),
  tail_rpc_client_(make_shared<RPCClient>(target_strs[target_strs.size() - 1])) {

}

//-----------------------------------------------------------------------------

void ChainClient::RunServer(string server_port) {
  client_rpc_server_ = make_shared<ClientRPCServer>(this);
  client_rpc_server_->RunServer(server_port);
}

//-----------------------------------------------------------------------------

void ChainClient::HandleReceiveRequest(const AckArg* ack_arg) {
  cout << "In chain client, received ack for key: "
       << ack_arg->key() << endl;
  operations_queue_.pop();
  keys_queue_.pop();
  values_queue_.pop();
  NextOperation();
}

//-----------------------------------------------------------------------------

void ChainClient::Put(string key, string value, string source_ip) {
  rpc_client_->Put(key, value, source_ip);
}

//-----------------------------------------------------------------------------

void ChainClient::Get(string key) {
  string val = tail_rpc_client_->Get(key);
  cout << "Got value " << val << endl;
  operations_queue_.pop();
  keys_queue_.pop();
  NextOperation();
}

//-----------------------------------------------------------------------------

void ChainClient::NextOperation() {
  if (operations_queue_.size() == 0) {
    cout << "All operations complete" << endl;
    end_time_ = std::chrono::high_resolution_clock::now();
    return;
  }

  string operation = operations_queue_.front();
  if (strcmp(operation.c_str(), "get") == 0) {
    Get(keys_queue_.front());
  } else if (strcmp(operation.c_str(), "put") == 0) {
    Put(keys_queue_.front(), values_queue_.front(), client_ip_);
  }
}


//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  vector<string> server_ips(2);

  ifstream config_file;
  config_file.open(argv[3]);

  cout << "input to client" << argv[1] << endl << argv[2] << endl << argv[3] << endl <<  argv[4] << endl;
  string config_line;
  string user = argv[4];
  if (config_file.is_open()) {
    while (config_file) {
      std::getline(config_file, config_line);

      if (config_line.size() == 0) {
        break;
      }
       // Get the id of first replica or head
       if (stoi(config_line.substr(0, config_line.find(","))) == 1) {
         server_ips[0] = config_line.substr(config_line.find(",") + 1);
       }
       if (stoi(config_line.substr(0, config_line.find(","))) == 3) {
         server_ips[1] = config_line.substr(config_line.find(",") + 1);
         break;
       }
    }
  } else {
      cout << "Could not open file";
  }

  ChainClient chain_client(server_ips);

  chain_client.client_ip_ = argv[1];

  string client_port = chain_client.client_ip_.substr
                                   (chain_client.client_ip_.find(":") + 1);

  thread t1(&ChainClient::RunServer, &chain_client, client_port);
  sleep(5);

  ifstream input_file;
  string input_file_path = "/home/" + user + "/chain-repl/inputs/write_workload/" + argv[2];
  cout << input_file_path << endl;
  input_file.open(input_file_path);

  string input_line;

  int counter = 0;
  if (input_file.is_open()) {
    while (input_file) {
      std::getline(input_file, input_line);
      istringstream ss(input_line);
      string word;
      while (ss >> word) {
        if (word.size() == 3) {
          chain_client.operations_queue_.push(word);
          counter++;
          cout << counter << endl;
        } else if (word.size() == 24) {
          chain_client.keys_queue_.push(word);
          counter++;
          cout << counter << endl;
        } else {
          chain_client.values_queue_.push(word);
          counter++;
          cout << counter << endl;
        }
      }
    }
    input_file.close();
  } else {
    cout << "cannot find input file, run random_gen file in benchmark" << endl;
  }

  cout << "Size of keys_queue_ is" << chain_client.keys_queue_.size() << endl;
  auto start=std::chrono::high_resolution_clock::now();;
  if (strcmp(chain_client.operations_queue_.front().c_str(), "put") == 0) {
    chain_client.Put(chain_client.keys_queue_.front(),
                     chain_client.values_queue_.front(),
                     chain_client.client_ip_);
  } else if (strcmp(chain_client.operations_queue_.front().c_str(), "get") == 0) {
    chain_client.Get(chain_client.keys_queue_.front());
  }

  auto elapsed = chain_client.end_time_ - start;
  long long microseconds = chrono::duration_cast<chrono::microseconds> 
                                                (elapsed).count();
  cout << "Time taken " << microseconds << endl;

  t1.join();
}
