#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/user_fills_parser.h"
#include "hyperliquid/trading/order_events.h"

#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 16> in{};
    event::ring_buffer<trading::order_event, 1024> out{};

    json::user_fills_parser<16, 1024> p(in, out);

    const char *sample = R"json(
{"channel":"userFills","data":[
  {"coin":"HYPE","side":"B","px":"22.46","sz":"1.5","oid":123,"cloid":"cid-001","time":1769339485000}
]}
)json";

    event::market_event me{};
    me.size = std::strlen(sample);
    std::memcpy(me.data.data(), sample, me.size);
    in.push(me);

    p.poll();

    std::size_t n = 0;
    trading::order_event ev{};
    while (out.pop(ev)) {
        ++n;
        if (ev.type == trading::order_event_type::fill) {
            std::printf("FILL cid=%s vid=%s side=%u px=%lld sz=%lld t=%llu\n",
                        ev.fill.client_id.value, ev.fill.venue_id.value, (unsigned)ev.fill.s,
                        (long long)ev.fill.fill_price, (long long)ev.fill.fill_qty,
                        (unsigned long long)ev.fill.exchange_time_ms);
        }
    }

    if (n != 1) {
        std::printf("FAIL emitted=%zu\n", n);
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}