#include "hyperliquid/ws/wss_client.h"

#include <cstdio>

int main() {
    using namespace hyperliquid;

    ws::wss_client c;
    if (!c.connect("api.hyperliquid.xyz", "/ws", 443)) {
        std::printf("FAIL connect/handshake\n");
        return 1;
    }

    // Subscribe to l2Book HYPE
    const char *sub =
        R"json({"method":"subscribe","subscription":{"type":"l2Book","coin":"HYPE"}})json";
    if (!c.send_text(sub)) {
        std::printf("FAIL send\n");
        return 2;
    }

    ws::text_view tv{};
    for (int i = 0; i < 2000; ++i) {
        auto st = c.poll(tv);
        if (st == ws::ws_status::ok) {
            std::printf("RX %.*s\n", (int)tv.size, tv.data);
            std::printf("PASS\n");
            return 0;
        }
        if (st == ws::ws_status::error || st == ws::ws_status::closed)
            break;
    }

    std::printf("FAIL no message\n");
    return 3;
}