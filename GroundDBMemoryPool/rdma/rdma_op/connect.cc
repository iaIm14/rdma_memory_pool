#include "lagacy.hh"
#include "rdma.hh"
#include <rdma.hh>

namespace mempool {

struct resources *
connectServer(const char *server_name, /* server host name */
              const int tcp_port,      /* server TCP port */
              const char *ib_devname,  /* server device name. If nullptr, client
                                          will use the first  found device */
              const int ib_port        /* server IB port */
) {
  auto *res = new struct resources();
  if (!res) {
    fprintf(stderr, "failed to malloc struct resource\n");
    return nullptr;
  }

  if (resources_create(res, server_name, ib_devname, ib_port)) {
    fprintf(stderr, "failed to create resources\n");
    return nullptr;
  }
  memory_region *memreg = nullptr;
  if (register_mr(memreg, res, nullptr, 1)) {
    fprintf(stderr, "failed to register memory regions\n");
    return nullptr;
  }
  connection *conn = nullptr;
  if (connect_qp(conn, res, memreg, server_name, tcp_port, -1, ib_port)) {
    fprintf(stderr, "failed to connect QPs\n");
    return nullptr;
  }
#ifdef GROUNDDB_MEMORY_POOL_DEBUG
  char *buf = new char[strlen(VERIFIER) + 1];
  rdma_read(res, memreg, conn, buf, strlen(VERIFIER) + 1);
  if (strcmp(memreg->buf, VERIFIER) != 0) {
    fprintf(stderr, "failed to verify connection\n");
    return nullptr;
  }
  fprintf(stdout, "Connection verified\n");
#endif
  return res;
}

} // namespace mempool