#include "lagacy.h"
#include <rdma.hh>

namespace mempool {

int rdma_write(const struct resources *res, /* RDMA Connection resources */
               const memory_region *memreg, const connection *conn,
               const char *buffer, /* Local buffer to write from */
               const size_t size   /* number of bytes to write */
) {
  // Attempt to perform rdma write
  memcpy(memreg->buf, buffer, size);
  if (post_send(res, memreg, conn, IBV_WR_RDMA_WRITE)) {
    fprintf(stderr, "failed to post SR\n");
    return 1;
  }

  if (poll_completion(res, conn)) {
    fprintf(stderr, "poll completion failed\n");
    return 1;
  }
  return 0;
}

} // namespace mempool