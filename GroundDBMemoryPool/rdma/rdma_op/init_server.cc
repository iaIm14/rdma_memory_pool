#include "lagacy.hh"
#include <rdma.hh>

namespace mempool {

resources *
init_server(const int tcp_port,     /* server TCP port */
            const char *ib_devname, /* server device name. If nullptr, client
                                       will use the first found device */
            const int ib_port       /* server IB port */
) {
  auto *res = new resources();
  if (!res) {
    fprintf(stderr, "failed to malloc struct resource\n");
    exit(1);
  }

  if (resources_create(res, nullptr, ib_devname, ib_port)) {
    fprintf(stderr, "failed to create resources\n");
    exit(1);
  }
  memory_region *memreg = nullptr;
  if (register_mr(memreg, res, nullptr, strlen(VERIFIER) + 1)) {
    fprintf(stderr, "failed to register memory regions\n");
    return nullptr;
  }
  connection *conn = nullptr;
  if (connect_qp(conn, res, memreg, nullptr, tcp_port, -1, ib_port)) {
    fprintf(stderr, "failed to connect QPs\n");
    exit(1);
  }
#ifdef GROUNDDB_MEMORY_POOL_DEBUG
  strcpy(memreg->buf, VERIFIER);
  fprintf(stdout, "going to send the message: '%s'\n", memreg->buf);
  if (rdma_write(res, memreg, conn, memreg->buf, strlen(VERIFIER) + 1)) {
    fprintf(stderr, "failed to write to remote memory\n");
    exit(1);
  }
#endif
  return res;
}

} // namespace mempool
