#include "rdma.hh"
namespace mempool {

connection::~connection() {
  if (qp)
    if (ibv_destroy_qp(qp)) {
      fprintf(stderr, "failed to destroy QP\n");
    }
  if (cq)
    if (ibv_destroy_cq(cq)) {
      fprintf(stderr, "failed to destroy CQ\n");
    }
  if (sock >= 0)
    if (close(sock)) {
      fprintf(stderr, "failed to close socket\n");
    }
}
void memory_region::deref() {
  if (!buf)
    return;
  auto pref = ref_.fetch_sub(1);
  if (pref == 1) {
    delete[] buf;
    buf = nullptr;
    size = 0;
  }
}
void memory_region::ref() { ref_.fetch_add(1); }
bool memory_region::alloc(size_t size) {
  if (buf)
    return false;
  buf = new char[size];
  this->size = size;
  if (!buf) {
    fprintf(stderr, "failed to allocate memory\n");
    exit(-1);
  }
  ref();
  return true;
}

bool memory_region::reg(const char *buf, size_t size) {
  if (this->buf)
    return false;
  this->buf = const_cast<char *>(buf);
  this->size = size;
  ref();
  ref();
  return true;
}

memory_region::memory_region(memory_region &&memreg) noexcept {
  buf = memreg.buf;
  size = memreg.size;
  memreg.buf = nullptr;
  memreg.size = 0;
  conn = memreg.conn;
  memreg.conn = nullptr;
  mr = memreg.mr;
  memreg.mr = nullptr;
}

memory_region::~memory_region() {
  if (conn)
    delete conn;
  deref();
  if (mr) {
    ibv_dereg_mr(mr);
    mr = nullptr;
  }
}

} // namespace mempool