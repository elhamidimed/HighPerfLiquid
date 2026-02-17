#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/order_updates_parser.h"
#include "hyperliquid/trading/order_events.h"

#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 16> in{};
    event::ring_buffer<trading::order_event, 1024> out{};

    json::order_updates_parser<16, 1024> p(in, out);

    // One "open" and one "rejected" example (matches WsOrder shape)
    const char *sample = R"json(
{"channel":"orderUpdates","data":[
  {"order":{"coin":"HYPE","side":"B","limitPx":"22.46","sz":"1.5","oid":123,"timestamp":1769339484000,"origSz":"1.5","cloid":"cid-001"},
   "status":"open","statusTimestamp":1769339484158},
  {"order":{"coin":"HYPE","side":"B","limitPx":"22.46","sz":"1.5","oid":124,"timestamp":1769339484000,"origSz":"1.5","cloid":"cid-002"},
   "status":"rejected","statusTimestamp":1769339484160}
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
        if (ev.type == trading::order_event_type::ack) {
            std::printf("ACK cid=%s vid=%s t=%llu\n", ev.ack.client_id.value, ev.ack.venue_id.value,
                        (unsigned long long)ev.ack.exchange_time_ms);
        } else if (ev.type == trading::order_event_type::reject) {
            std::printf("REJECT cid=%s reason=%u msg=%s t=%llu\n", ev.reject.client_id.value,
                        (unsigned)ev.reject.reason, ev.reject.message,
                        (unsigned long long)ev.reject.exchange_time_ms);
        }
    }

    if (n != 2) {
        std::printf("FAIL emitted=%zu\n", n);
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}