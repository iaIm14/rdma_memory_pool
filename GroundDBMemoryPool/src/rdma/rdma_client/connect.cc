#include "../util.h"
#include <rdma.hh>

namespace mempool {

struct resources *
connectServer(const char *server_name, /* server host name */
              const int tcp_port,      /* server TCP port */
              const char *ib_devname,  /* server device name. If nullptr, client
                                          will use the first  found device */
              const int ib_port        /* server IB port */
) {
  struct resources *res = new struct resources();
  if (!res) {
    fprintf(stderr, "failed to malloc struct resource\n");
    return nullptr;
  }

  if (resources_create(res, server_name, ib_devname, ib_port)) {
    fprintf(stderr, "failed to create resources\n");
    return nullptr;
  }
  struct memory_region *memreg = nullptr;
  if (register_mr(memreg, res)) {
    fprintf(stderr, "failed to register memory regions\n");
    return nullptr;
  }
  struct connection *conn = nullptr;
  if (connect_qp(conn, res, memreg, server_name, tcp_port, -1, ib_port)) {
    fprintf(stderr, "failed to connect QPs\n");
    return nullptr;
  }
  // Following lines are only for simulation and will be removed in the future.
  if (post_receive(res, memreg, conn)) {
    fprintf(stderr, "failed to post RR\n");
    return nullptr;
  }
  if (poll_completion(res, &res->memregs[0].conns[0])) {
    fprintf(stderr, "poll completion failed\n");
    return nullptr;
  }
  if (strcmp(res->memregs[0].buf, VERIFIER)) {
    fprintf(stderr, "failed to verify connection\n");
    return nullptr;
  }
  fprintf(stdout, "Connection verified\n");
  // Remove until here
  return res;
}

} // namespace mempool