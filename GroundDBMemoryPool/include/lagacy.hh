#pragma once

#include "rdma.hh"
namespace mempool {

int sock_connect(const char *serverName, int port);
int poll_completion(const struct connection *conn);
int post_send(const struct resources *res, const struct memory_region *memregs,
              const struct connection *conn, const enum ibv_wr_opcode opcode);
int post_receive(const struct memory_region *memregs,
                 const struct connection *conn);
int post_receive(const struct resources *res,
                 const struct memory_region *memregs,
                 const struct connection *conn);
int resources_create(struct resources *res, const char *serverName,
                     const char *ibDevName, const int ibPort);
int register_mr(memory_region *&memreg, struct resources *res,
                const char *buf = nullptr, size_t size = -1);
int connect_qp(connection *&conn, struct resources *res, memory_region *memreg,
               const char *serverName, const int tcpPort, const int gid_idx,
               const int ibPort);
int resources_destroy(struct resources *res);

} // namespace mempool