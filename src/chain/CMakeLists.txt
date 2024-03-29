cmake_minimum_required(VERSION 3.5.1)

project(chain C CXX)

include(../cmake/common.cmake)

# Proto file
get_filename_component(chain_proto "../protos/chain.proto" ABSOLUTE)
get_filename_component(chain_proto_path "${chain_proto}" PATH)

# Generated sources
set(chain_srcs "${CMAKE_CURRENT_BINARY_DIR}/chain.pb.cc")
set(chain_hdrs "${CMAKE_CURRENT_BINARY_DIR}/chain.pb.h")
set(chain_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/chain.grpc.pb.cc")
set(chain_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/chain.grpc.pb.h")

add_custom_command(
      OUTPUT "${chain_srcs}" "${chain_hdrs}" "${chain_grpc_srcs}" "${chain_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${chain_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${chain_proto}"
      DEPENDS "${chain_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(chain_proto
  ${chain_srcs}
  ${chain_hdrs}
  ${chain_grpc_srcs}
  ${chain_grpc_hdrs})
target_link_libraries(chain_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# RPC Client
add_library(rpc_client rpc_client.cc)
add_library(rpc_server rpc_server.cc)
add_library(client_rpc_server client_rpc_server.cc)
foreach(_target
  rpc_client rpc_server client_rpc_server)
  target_link_libraries(${_target}
    chain_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

foreach(_target
  chain_replica)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    rpc_server
    rpc_client
    chain_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

foreach(_target
  chain_client)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    client_rpc_server
    rpc_client
    chain_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
