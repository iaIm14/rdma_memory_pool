#include "rdma.hh"
int main() {
  mempool::connectServer("127.0.0.1", 122189, nullptr, 1);
  return 0;
}