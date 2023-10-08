cmake_minimum_required(VERSION 3.14)
project(GroundDBMemoryPool)

option(WITH_DEBUG "build debug" OFF)
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
  memory_pool_client main_client.cc
)

# An example of a server that uses the memory pool
add_executable(
  memory_pool_server main_server.cc
)

# Static library used by client to interact with server
add_library(
  rdma_op
  rdma/rdma.cc
  util/util.cc
  rdma/rdma_op/connect.cc
  rdma/rdma_op/read.cc
  rdma/rdma_op/write.cc
  rdma/rdma_op/flush.cc
  rdma/rdma_op/init_server.cc
)
target_include_directories(rdma_op PRIVATE ${CMAKE_SOURCE_DIR}/util)
target_include_directories(rdma_op PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_link_libraries(rdma_op PRIVATE ibverbs)
target_link_libraries(memory_pool_client PRIVATE ibverbs rdma_op)
target_link_libraries(memory_pool_server PRIVATE ibverbs rdma_op)

# Compile with -DSERVER for server
target_compile_definitions(
  memory_pool_server PRIVATE SERVER
)
if(WITH_DEBUG)
  add_definitions(-D_GROUNDDB_MEMORY_POOL_DEBUG)
endif()
