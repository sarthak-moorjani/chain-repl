/*
 * Authors: sm106@illinois.edu, shubhij2@illinois.edu
 *
 *
 */

#include "chain_client.h"
#include "client_rpc_server.h"

#include "chain.grpc.pb.h"

#include <assert.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <thread>
#include <time.h>
#include <unistd.h>

using chain::ChainImpl;
using chain::AckArg;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

//-----------------------------------------------------------------------------

ChainClient::ChainClient(vector<string> target_strs, vector<int> target_ids, int head_id) :
  next_ops_ctr_(0),
  head_replica_id_(head_id) {

  for (int i = 0; i < target_ids.size(); i++) {
    replica_map_[target_ids[i]] = make_shared<RPCClient>(target_strs[i]);
  }
}

//-----------------------------------------------------------------------------

void ChainClient::RunServer(string server_port) {
  client_rpc_server_ = make_shared<ClientRPCServer>(this);
  client_rpc_server_->RunServer(server_port);
}

//-----------------------------------------------------------------------------

void ChainClient::HandleReceiveRequest(const AckArg* ack_arg) {
  //cout << "received ack" << endl;
  next_ops_ctr_++;
  auto end = std::chrono::high_resolution_clock::now();
  pair<nowtime, nowtime> p = put_latency_tracker_[ack_arg->key()];
  put_latency_tracker_[ack_arg->key()] = make_pair(p.first, end);
}

//-----------------------------------------------------------------------------

void ChainClient::Put(string key, string value, string source_ip) {
  auto start = std::chrono::high_resolution_clock::now();
  put_latency_tracker_[key] = make_pair(start, start);
  replica_map_[head_replica_id_]->Put(key, value, source_ip);
}

//-----------------------------------------------------------------------------

void ChainClient::Get(string key, int replica_id) {
  //srand(time(NULL));
  auto start = std::chrono::high_resolution_clock::now();
  string val = replica_map_[replica_id]->Get(key);
  auto end = std::chrono::high_resolution_clock::now();
  if (val.empty()) {
    key_counter_++;
  }
  get_latency_tracker_[key] = make_pair(start, end);
  cout << "Received val : " << val << endl;
  next_ops_ctr_++;
}

//-----------------------------------------------------------------------------

void ChainClient::NextOperation() {
  int replica_id = 0;
  while (1) {
    if (operations_queue_.size() == 0) {
      end_time_ = std::chrono::high_resolution_clock::now();
      cout << "All operations complete" << endl;
      long long total_microseconds = 0;
      for (auto t : put_latency_tracker_) {
        auto elapsed = t.second.second - t.second.first;
        long long microseconds = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        total_microseconds += microseconds;
        // cout << microseconds << endl;
      }
      cout << "Average for put is" << total_microseconds/100 << endl;

      total_microseconds = 0;
      for (auto t : get_latency_tracker_) {
        auto elapsed = t.second.second - t.second.first;
        long long microseconds = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        total_microseconds += microseconds;
        //cout << microseconds << endl;
      }
      cout << "Average for get is" << total_microseconds/100;
      return;
    }
    if (next_ops_ctr_ > 0) {
      next_ops_ctr_--;
      string operation = operations_queue_.front();
      operations_queue_.pop();
      if (strcmp(operation.c_str(), "get") == 0) {
        string key = keys_queue_.front();
        keys_queue_.pop();
        replica_id = 1 + (replica_id + 1)%3;
        Get(key, replica_id);
      } else if (strcmp(operation.c_str(), "put") == 0) {
        string key = keys_queue_.front();
        keys_queue_.pop();
        string value = values_queue_.front();
        values_queue_.pop();
        Put(key, value, client_ip_);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void ChainClient::InitQueue(string file_path) {
  ifstream input_file;
  input_file.open(file_path);

  string input_line;

  int counter = 0;
  if (input_file.is_open()) {
    while (input_file) {
      std::getline(input_file, input_line);
      istringstream ss(input_line);
      string word;
      //if (counter == 5000)
      //  break;
      while (ss >> word) {
        if (word.size() == 3) {
          operations_queue_.push(word);
          counter++;
        } else if (word.size() == 24) {
          keys_queue_.push(word);
        } else {
          values_queue_.push(word);
        }
      }
    }
    input_file.close();
  } else {
    cout << "cannot find input file, run random_gen file in benchmark" << endl;
  }
}

//-----------------------------------------------------------------------------

void ChainClient::FirstCall() {
  if (strcmp(operations_queue_.front().c_str(), "put") == 0) {
    Put(keys_queue_.front(), values_queue_.front(),client_ip_);
  } else if (strcmp(operations_queue_.front().c_str(), "get") == 0) {
    Get(keys_queue_.front(), 1);
  }
}

//-----------------------------------------------------------------------------

void ChainClient::TestMethod(string op) {
  static const char alpha[] = "abcdefghijklmnopqrstuvwxyz";
  string key;
  string value;
  vector<string> keys;
  key.reserve(24);
  value.reserve(10);
  for (int j = 1; j <= 100; j++) {
    operations_queue_.push(op);
    if (strcmp(op.c_str(), "get") == 0) {
      continue;
    }
    for (int i = 0; i < 24; i++) {
      key += alpha[rand() % (sizeof(alpha) - 1)];
    }
    if (strcmp(op.c_str(), "put") == 0) {
      for (int i = 0; i < 10; i++) {
        value += alpha[rand() % (sizeof(alpha) - 1)];
      }
      values_queue_.push(value);
      value.resize(0);
      keys_queue_.push(key);
      keys.push_back(key);
      key.resize(0);
    }
  }
  if (strcmp(op.c_str(), "put") == 0) {
    for (int j = 0; j < 100; j++) {
      keys_queue_.push(keys[j]);
    }
  }
}
//-----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  vector<string> replica_ips;
  std::vector<int> replica_ids;

  ifstream config_file;
  config_file.open(argv[3]);
  int min_id = INT_MAX;

  cout << "input to client" << argv[1] << endl << argv[2] << endl << argv[3] << endl <<  argv[4] << endl;
  string config_line;
  string user = argv[4];

  if (config_file.is_open()) {
  	  while (config_file) {
  	    std::getline(config_file, config_line);

        if (config_line.size() == 0) {
          break;
        }
  		// Get the id of all the replicas
  	    int input_id = stoi((config_line.substr(0, config_line.find(","))));
  		min_id = min(min_id, input_id);
  		replica_ids.push_back(input_id);
  		replica_ips.push_back(config_line.substr(config_line.find(",") + 1));
      }
  	} else {
      cout << "Could not open file";
    }

  ChainClient chain_client(replica_ips, replica_ids, min_id);

  chain_client.client_ip_ = argv[1];

  string client_port = chain_client.client_ip_.substr
                                   (chain_client.client_ip_.find(":") + 1);

  thread t1(&ChainClient::RunServer, &chain_client, client_port);
  sleep(5);

  string input_file_path = "/users/" + user + "/chain-repl/inputs/write_workload/" + argv[2];
  cout << input_file_path << endl;
  chain_client.InitQueue(input_file_path);
  //chain_client.TestMethod("put");

  chain_client.key_counter_ = 0;

  cout << "Size of keys_queue_ is" << chain_client.keys_queue_.size() << endl;
  auto start_put=std::chrono::high_resolution_clock::now();
  chain_client.FirstCall();

  thread t2(&ChainClient::NextOperation, &chain_client);
  t2.join();
  cout << "hello 1" << endl; 
  auto elapsed_put = chain_client.end_time_ - start_put;
  long long microseconds = chrono::duration_cast<chrono::microseconds>(elapsed_put).count();
  cout << "Time taken for put " << microseconds << endl;

  string write_string = "write";
    int str_index;
    while((str_index = input_file_path.find(write_string)) != string::npos) {
        input_file_path.replace(str_index, write_string.length(), "read");
    }

  chain_client.InitQueue(input_file_path);
  //chain_client.TestMethod("get");
  cout << "Size of keys_queue_ is" << chain_client.keys_queue_.size() << endl;
  auto start_get = std::chrono::high_resolution_clock::now();
  chain_client.FirstCall();

  thread t3(&ChainClient::NextOperation, &chain_client);
  t3.join();
  cout << "hello 2" << endl;
  auto elapsed_get = chain_client.end_time_ - start_get;
  microseconds = chrono::duration_cast<chrono::microseconds>(elapsed_get).count();
  cout << "Time taken for get " << microseconds << endl;
  cout << "Keys not found " << chain_client.key_counter_ << endl;

  t1.join();
}
