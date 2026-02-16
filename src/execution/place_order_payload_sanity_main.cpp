#include "hyperliquid/execution/hyperliquid_place_order_payload.h"
#include "hyperliquid/market/symbol.h"
#include "hyperliquid/trading/order_request.h"
#include "hyperliquid/trading/validate.h"

#include <array>
#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    trading::order_request r{};
    market::set_symbol(r.symbol, "HYPE");
    r.s = trading::side::buy;
    r.type = trading::order_type::limit;
    r.tif = trading::time_in_force::gtc;
    r.post_only = 1;
    r.reduce_only = 0;
    r.price = 22'460'000;
    r.qty = 1'500'000;
    std::memcpy(r.client_id.value, "cid-001", 7);

    if (trading::validate(r) != trading::validate_error::ok) {
        std::printf("FAIL: validate\n");
        return 1;
    }

    std::array<char, 4096> buf{};
    std::size_t used = 0;

    // Placeholder signature object (caller-provided for now)
    const char *sig = R"json({"r":"00","s":"00","v":27})json";

    const bool ok = execution::build_place_order_ws_post(buf.data(), buf.size(), used,
                                                         /*request_id*/ 1,
                                                         /*nonce*/ 1769339484158ULL,
                                                         /*asset*/ 0, r, sig);

    if (!ok || used == 0) {
        std::printf("FAIL: build\n");
        return 2;
    }

    std::printf("%.*s\n", (int)used, buf.data());
    std::printf("PASS\n");
    return 0;
}