#include "hyperliquid/config/config.h"
#include "hyperliquid/config/config_loader.h"
#include "hyperliquid/market/symbol.h"
#include "hyperliquid/risk/from_config.h"
#include "hyperliquid/risk/risk_engine.h"
#include <cstring>

#include <cstdio>

int main() {
    using namespace hyperliquid;

    // I'll change later to load an actual file
    config::library_config cfg = config::load_from_file("dummy.json");
    risk::risk_limits lim = risk::from_config(cfg);

    risk::risk_engine eng(lim);

    // Building a valid order request
    trading::order_request ok{};
    market::set_symbol(ok.symbol, "HYPE");
    ok.s = trading::side::buy;
    ok.type = trading::order_type::limit;
    ok.tif = trading::time_in_force::gtc;
    ok.post_only = 0;
    ok.reduce_only = 0;
    ok.price = 22'460'000;
    ok.qty = 1'000'000;
    std::memcpy(ok.client_id.value, "cid-001", 7);

    // Allow with default limits
    auto d0 = eng.check(ok);
    if (!d0.allow) {
        std::printf("FAIL expected allow d0 reason=%u\n", (unsigned)d0.reason);
        return 2;
    }

    // Now enforcing post only via local limits
    risk::risk_limits lim2 = lim;
    lim2.require_post_only = 1;
    risk::risk_engine eng2(lim2);

    auto d1 = eng2.check(ok);
    if (d1.allow || d1.reason != risk::decision_reason::post_only_required) {
        std::printf("FAIL expected post only deny allow=%u reason=%u\n", (unsigned)d1.allow,
                    (unsigned)d1.reason);
        return 3;
    }

    // checking qty limit
    risk::risk_limits lim3 = lim;
    lim3.max_order_qty = 500'000; // 0.5
    risk::risk_engine eng3(lim3);

    auto d2 = eng3.check(ok);
    if (d2.allow || d2.reason != risk::decision_reason::qty_limit) {
        std::printf("FAIL expected qty deny allow=%u reason=%u\n", (unsigned)d2.allow,
                    (unsigned)d2.reason);
        return 4;
    }

    std::printf("PASS\n");
    return 0;

    std::printf("risk: max_qty=%lld max_notional=%lld require_post_only=%u allow_market=%u\n",
                (long long)lim.max_order_qty, (long long)lim.max_order_notional,
                (unsigned)lim.require_post_only, (unsigned)lim.allow_market_orders);

    std::printf("PASS\n");
    return 0;
}