cmake_minimum_required(VERSION 3.14)
project(GroundDBMemoryPool)

add_definitions(-D_GROUNDDB_MEMORY_POOL_DEBUG false)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/include/rdma)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Use verbose output
set(CMAKE_VERBOSE_MAKEFILE ON)

include(GoogleTest)

enable_testing()

# An example of a client that uses the memory pool
add_executable(
  memory_pool_client
  src/main_client.cc
)

# An example of a server that uses the memory pool
add_executable(
  memory_pool_server
  src/main_server.cc
)

# Static library used by client to interact with server
add_library(
  rdma_client_lib
  src/rdma/rdma_client/connect.cc
  src/rdma/util.cc
  src/rdma/rdma_client/read.cc
  src/rdma/rdma_client/write.cc
  src/rdma/rdma_client/flush.cc
)

# Static library used by server
add_library(
  rdma_server_lib
  src/rdma/rdma_server/init_server.cc
  src/rdma/util.cc
)

# Compile with -DSERVER for server
target_compile_definitions(
  memory_pool_server PRIVATE SERVER
)

target_link_libraries(rdma_client_lib PRIVATE ibverbs)
target_link_libraries(memory_pool_client PRIVATE ibverbs rdma_client_lib)

target_link_libraries(rdma_server_lib PRIVATE ibverbs)
target_link_libraries(memory_pool_server PRIVATE ibverbs rdma_server_lib)

