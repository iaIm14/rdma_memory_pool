#pragma once
#include "rdma.hh"
#include <cstdio>
// Server
namespace mempool {
struct resources *init_server(
    const int tcp_port,     /* server TCP port */
    const char *ib_devname, /* server device name. If nullptr, client
                                         will use the first found device */
    const int ib_port       /* server IB port */
);
}