#include "hyperliquid/config/config.h"
#include "hyperliquid/execution/submit_order.h"
#include "hyperliquid/market/symbol.h"
#include "hyperliquid/risk/from_config.h"
#include "hyperliquid/risk/risk_engine.h"
#include "hyperliquid/trading/order_request.h"

#include <array>
#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    config::library_config cfg{};
    risk::risk_limits lim = risk::from_config(cfg);

    // Allow engine
    risk::risk_engine allow_eng(lim);

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

    std::array<char, 4096> buf{};
    const char *sig = R"json({"r":"00","s":"00","v":27})json";

    auto ok =
        execution::submit_order(allow_eng, buf.data(), buf.size(), 1, 1769339484158ULL, 0, r, sig);

    if (!ok.decision.allow || ok.json_bytes == 0) {
        std::printf("FAIL expected allowed+json allow=%u reason=%u bytes=%zu\n",
                    (unsigned)ok.decision.allow, (unsigned)ok.decision.reason, ok.json_bytes);
        return 1;
    }

    std::printf("ALLOWED bytes=%zu\n", ok.json_bytes);

    // Deny engine: require post-only but set order to non-post-only
    risk::risk_limits lim2 = lim;
    lim2.require_post_only = 1;
    risk::risk_engine deny_eng(lim2);

    trading::order_request bad = r;
    bad.post_only = 0;

    std::memset(buf.data(), 0, buf.size());

    auto den =
        execution::submit_order(deny_eng, buf.data(), buf.size(), 2, 1769339484159ULL, 0, bad, sig);

    if (den.decision.allow || den.json_bytes != 0) {
        std::printf("FAIL expected denied+nojson allow=%u reason=%u bytes=%zu\n",
                    (unsigned)den.decision.allow, (unsigned)den.decision.reason, den.json_bytes);
        return 2;
    }

    if (den.decision.reason != risk::decision_reason::post_only_required) {
        std::printf("FAIL expected post_only_required reason=%u\n", (unsigned)den.decision.reason);
        return 3;
    }

    std::printf("DENIED reason=%u\n", (unsigned)den.decision.reason);
    std::printf("PASS\n");
    return 0;
}