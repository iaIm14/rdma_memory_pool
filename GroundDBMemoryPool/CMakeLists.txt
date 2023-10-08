cmake_minimum_required(VERSION 3.14)
project(GroundDBMemoryPool)

# add_definitions(-D_GROUNDDB_MEMORY_POOL_DEBUG false)
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
  rdma_client
  rdma/rdma.cc
  util/util.cc
  rdma/rdma_client/connect.cc
  rdma/rdma_client/read.cc
  rdma/rdma_client/write.cc
  rdma/rdma_client/flush.cc
)
target_include_directories(rdma_client PRIVATE ${CMAKE_SOURCE_DIR}/util)
target_include_directories(rdma_client PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Static library used by server
add_library(
  rdma_server
  rdma/rdma.cc
  util/util.cc
  rdma/rdma_server/init_server.cc
)
target_include_directories(rdma_server PRIVATE ${CMAKE_SOURCE_DIR}/util)
target_include_directories(rdma_server PUBLIC ${CMAKE_SOURCE_DIR}/include)
# Compile with -DSERVER for server
# target_compile_definitions(
#   memory_pool_server PRIVATE SERVER
# )

target_link_libraries(rdma_client PRIVATE ibverbs)
target_link_libraries(memory_pool_client PRIVATE ibverbs rdma_client)


target_link_libraries(rdma_server PRIVATE ibverbs)
target_link_libraries(memory_pool_server PRIVATE ibverbs rdma_server)

