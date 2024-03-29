#include "lagacy.hh"
#include <rdma.hh>

namespace mempool {

int rdma_read(const struct resources *res, /* RDMA Connection resources */
              const memory_region *memreg, const connection *conn,
              char *buffer,     /* Local buffer to read into */
              const size_t size /* number of bytes to read */
) {
  if (post_send(res, memreg, conn, IBV_WR_RDMA_READ)) {
    fprintf(stderr, "failed to post SR\n");
    return 1;
  }

  if (poll_completion(conn)) {
    fprintf(stderr, "poll completion failed\n");
    return 1;
  }
  memcpy(buffer, memreg->buf, size);
  return 0;
}

} // namespace mempool