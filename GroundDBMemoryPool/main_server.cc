#include "rdma.hh"
int main() {
  mempool::init_server(122189, nullptr, 1);
  return 0;
}
