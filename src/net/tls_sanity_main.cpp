#include "hyperliquid/net/tls_socket.h"

#include <cstdio>

int main() {
    using namespace hyperliquid;

    net::tls_socket s;
    auto st = s.connect("api.hyperliquid.xyz", 443);
    if (st != net::net_status::ok) {
        std::printf("FAIL connect\n");
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}