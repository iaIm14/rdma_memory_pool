#include "rdma.hh"
#include "rdma_server.hh"
int main() {
  mempool::init_server(122189, nullptr, 1);
  return 0;
}
