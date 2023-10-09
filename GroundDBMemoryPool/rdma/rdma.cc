#include "rdma.hh"
#include "lagacy.hh"
#include "rdma_type.hh"
#include <cassert>
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

RDMABase::RDMABase(const char *server_name, const char *dev_name,
                   const int ib_port) {
  if (resources_create(res, server_name, dev_name, ib_port)) {
    fprintf(stderr, "failed to create resources\n");
    exit(1);
  }
}

bool RDMANode::send(const void *buf, size_t size) {
  if (register_mr(memreg_, base_->res, reinterpret_cast<const char *>(buf),
                  size)) {
    fprintf(stderr, "failed to register memory regions\n");
    return false;
  }
  if (connect_qp(conn_, base_->res, memreg_, server_name_, tcp_port_, -1,
                 ib_port_)) {
    fprintf(stderr, "failed to connect QPs\n");
    return false;
  }
  if (post_send(base_->res, memreg_, conn_, IBV_WR_RDMA_WRITE)) {
    fprintf(stderr, "failed to post SR\n");
    return false;
  }
  if (poll_completion(conn_)) {
    fprintf(stderr, "poll completion failed\n");
    return false;
  }
  return true;
}

bool RDMANode::receive(void *buf, size_t size) {
  if (buf == nullptr || size == 0) {
    fprintf(stderr, "buffer is nullptr or size is 0\n");
    return false;
  }
  if (register_mr(memreg_, base_->res, reinterpret_cast<char *>(buf), size)) {
    fprintf(stderr, "failed to register memory regions\n");
    return false;
  }
  if (connect_qp(conn_, base_->res, memreg_, server_name_, tcp_port_, -1,
                 ib_port_)) {
    fprintf(stderr, "failed to connect QPs\n");
    return false;
  }
  if (post_receive(base_->res, memreg_, conn_)) {
    fprintf(stderr, "failed to post receive\n");
    return false;
  }
  if (poll_completion(conn_)) {
    fprintf(stderr, "poll completion failed\n");
    return false;
  }
  return true;
}

bool RDMANode::receive_meta(size_t *size) {
  if (size == nullptr) {
    fprintf(stderr, "size is nullptr\n");
    return false;
  }
  size_t recv_size = 0;
  int read_bytes = 0;
  read_bytes = read(conn_->sock, &recv_size, sizeof(size_t));
  if (read_bytes != sizeof(size_t)) {
    fprintf(stderr, "failed to read size\n");
    return false;
  }
  if (*size == 0) {
    *size = recv_size;
  } else if (*size != recv_size) {
    fprintf(stderr, "size is not equal\n");
    return false;
  }
  return true;
}

bool RDMANode::send_meta(size_t size) {
  if (write(conn_->sock, &size, sizeof(size_t)) != sizeof(size_t)) {
    fprintf(stderr, "failed to write size\n");
    return false;
  }
  return true;
}

} // namespace mempool