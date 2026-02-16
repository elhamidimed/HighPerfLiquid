#include "hyperliquid/market/symbol.h"
#include "hyperliquid/trading/order_events.h"
#include "hyperliquid/trading/order_request.h"
#include "hyperliquid/trading/validate.h"

#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    trading::order_request req{};
    market::set_symbol(req.symbol, "HYPE");
    req.s = trading::side::buy;
    req.type = trading::order_type::limit;
    req.tif = trading::time_in_force::gtc;
    req.post_only = 1;
    req.reduce_only = 0;
    req.price = 22'460'000;
    req.qty = 1'000'000;
    std::memcpy(req.client_id.value, "cid-001", 7);

    auto err = trading::validate(req);
    if (err != trading::validate_error::ok) {
        std::printf("validate failed: %u\n", (unsigned)err);
        return 1;
    }

    trading::order_request bad = req;
    bad.type = trading::order_type::market;
    bad.price = 1; // invalid by our rule
    auto err2 = trading::validate(bad);
    if (err2 == trading::validate_error::ok) {
        std::printf("expected validation failure for bad market order\n");
        return 2;
    }

    trading::order_event ev{};
    ev.type = trading::order_event_type::ack;
    std::memcpy(ev.ack.client_id.value, req.client_id.value, sizeof(req.client_id.value));
    std::memcpy(ev.ack.venue_id.value, "vid-abc", 7);
    ev.ack.exchange_time_ms = 123456;

    std::printf(
        "order_request: sym=%s side=%u type=%u tif=%u px=%lld qty=%lld post=%u reduce=%u cid=%s\n",
        req.symbol.value, (unsigned)req.s, (unsigned)req.type, (unsigned)req.tif,
        (long long)req.price, (long long)req.qty, (unsigned)req.post_only,
        (unsigned)req.reduce_only, req.client_id.value);

    std::printf("order_event(ack): cid=%s vid=%s t=%llu\n", ev.ack.client_id.value,
                ev.ack.venue_id.value, (unsigned long long)ev.ack.exchange_time_ms);

    return 0;
}