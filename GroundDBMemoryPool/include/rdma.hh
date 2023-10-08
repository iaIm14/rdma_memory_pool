#pragma once
#include <atomic>
#include <byteswap.h>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <endian.h>
#include <getopt.h>
#include <unistd.h>
#include <vector>

#include <arpa/inet.h>
#include <infiniband/verbs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

namespace mempool {

/* poll CQ timeout in millisec (2 seconds) */
#define MAX_POLL_CQ_TIMEOUT 2000
#define VERIFIER "VERIFY_CONNECTION"
#define RDMAMSGR "RDMA read operation "
#define RDMAMSGW "RDMA write operation"
#define BUFFER_SIZE 1024 * 1024 /* 1024KB*/
#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t htonll(uint64_t x) { return bswap_64(x); }
static inline uint64_t ntohll(uint64_t x) { return bswap_64(x); }
#elif __BYTE_ORDER == __BIG_ENDIAN
static inline uint64_t htonll(uint64_t x) { return x; }
static inline uint64_t ntohll(uint64_t x) { return x; }
#else
#error __BYTE_ORDER is neither __LITTLE_ENDIAN nor __BIG_ENDIAN
#endif

/* structure to exchange data which is needed to connect the QPs */
struct cm_con_data_t {
  uint64_t addr;   /* Buffer address */
  uint32_t rkey;   /* Remote key */
  uint32_t qp_num; /* QP number */
  uint16_t lid;    /* LID of the IB port */
  uint8_t gid[16]; /* gid */
} __attribute__((packed));

class connection {
public:
  struct ibv_cq *cq{nullptr}; /* CQ handle */
  struct ibv_qp *qp{nullptr}; /* QP handle */
  int sock{-1};               /* TCP socket file descriptor */
  ~connection() noexcept;
};

// we make sure no memory leak. buf read/write is not thread safe
class memory_region {
public:
  connection *conn{nullptr};    /* connection handle */
  struct ibv_mr *mr{nullptr};   /* MR handle for buf */
  std::atomic<int32_t> ref_{0}; /* reference count */
  char *buf{nullptr}; /* memory buffer pointer, used for RDMA and send ops */
  size_t size{0};     /* memory buffer size */
  bool alloc(size_t size);
  bool reg(const char *buf, size_t size);
  void ref();
  void deref();

public:
  memory_region() = default;
  ~memory_region();
  memory_region(const memory_region &) = delete;
  memory_region(memory_region &&memreg) noexcept;
};
/* structure of system resources */
class resources {
public:
  struct ibv_device_attr device_attr;
  /* Device attributes */
  struct ibv_port_attr port_attr;    /* IB port attributes */
  struct cm_con_data_t remote_props; /* values to connect to remote side */
  struct ibv_context *ib_ctx;        /* device handle */
  struct ibv_pd *pd;                 /* PD handle */
};

} // namespace mempool
