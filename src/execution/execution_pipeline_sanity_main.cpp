#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/execution/execution_dispatcher.h"
#include "hyperliquid/json/order_updates_parser.h"
#include "hyperliquid/json/user_fills_parser.h"
#include "hyperliquid/trading/order_events.h"

#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    // Raw input from WS
    event::ring_buffer<event::market_event, 16> raw_in{};

    // Fanout queues (one per parser)
    event::ring_buffer<event::market_event, 16> updates_in{};
    event::ring_buffer<event::market_event, 16> fills_in{};

    // Unified typed output
    event::ring_buffer<trading::order_event, 1024> out{};

    execution::execution_dispatcher<16, 16, 16> disp(raw_in, updates_in, fills_in);
    json::order_updates_parser<16, 1024> upd_parser(updates_in, out);
    json::user_fills_parser<16, 1024> fills_parser(fills_in, out);

    const char *msg_updates = R"json(
{"channel":"orderUpdates","data":[
  {"order":{"coin":"HYPE","side":"B","limitPx":"22.46","sz":"1.5","oid":123,"timestamp":1769339484000,"origSz":"1.5","cloid":"cid-001"},
   "status":"open","statusTimestamp":1769339484158}
]}
)json";

    const char *msg_fills = R"json(
{"channel":"userFills","data":[
  {"coin":"HYPE","side":"B","px":"22.46","sz":"1.5","oid":123,"cloid":"cid-001","time":1769339485000}
]}
)json";

    // Pushing both into raw input
    {
        event::market_event me{};
        me.size = std::strlen(msg_updates);
        std::memcpy(me.data.data(), msg_updates, me.size);
        raw_in.push(me);
    }
    {
        event::market_event me{};
        me.size = std::strlen(msg_fills);
        std::memcpy(me.data.data(), msg_fills, me.size);
        raw_in.push(me);
    }

    // Running dispatcher + parsers
    disp.poll();
    upd_parser.poll();
    fills_parser.poll();

    // Drain unified output
    std::size_t ack = 0, fill = 0;
    trading::order_event ev{};
    while (out.pop(ev)) {
        if (ev.type == trading::order_event_type::ack) {
            ++ack;
            std::printf("ACK cid=%s vid=%s t=%llu\n", ev.ack.client_id.value, ev.ack.venue_id.value,
                        (unsigned long long)ev.ack.exchange_time_ms);
        } else if (ev.type == trading::order_event_type::fill) {
            ++fill;
            std::printf("FILL cid=%s vid=%s side=%u px=%lld sz=%lld t=%llu\n",
                        ev.fill.client_id.value, ev.fill.venue_id.value, (unsigned)ev.fill.s,
                        (long long)ev.fill.fill_price, (long long)ev.fill.fill_qty,
                        (unsigned long long)ev.fill.exchange_time_ms);
        } else {
            std::printf("OTHER type=%u\n", (unsigned)ev.type);
        }
    }

    if (ack != 1 || fill != 1) {
        std::printf("FAIL ack=%zu fill=%zu\n", ack, fill);
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}