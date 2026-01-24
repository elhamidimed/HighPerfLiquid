#include "hyperliquid/net/tcp_socket.h"
#include <iostream>

void example_socket() {
    hyperliquid::net::tcp_socket sock;
    if (sock.connect("127.0.0.1", 1234) == hyperliquid::net::net_status::ok) {
        char buf[1024];
        std::size_t read_bytes;
        sock.read(buf, sizeof(buf), read_bytes);
    }
}
