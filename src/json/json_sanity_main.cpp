#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/l2_book_parser.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <simdjson.h>

static bool push_msg(hyperliquid::event::ring_buffer<hyperliquid::event::market_event, 1024> &in,
                     const char *s) {
    hyperliquid::event::market_event me{};
    me.size = std::strlen(s);
    std::memcpy(me.data.data(), s, me.size);
    return in.push(me);
}

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 1024> in{};
    event::ring_buffer<event::l2_level_event, 65536> out{};

    json::l2_book_parser<1024, 65536> parser(in, out);

    const char *sample =
        "{\"channel\":\"l2Book\",\"data\":{\"coin\":\"HYPE\",\"time\":1769339484158,"
        "\"levels\":[[{\"px\":\"22.46\",\"sz\":\"265.68\",\"n\":5}],"
        "[{\"px\":\"22.461\",\"sz\":\"62.37\",\"n\":2}]]}}";

    const char *wrong_channel = "{\"channel\":\"trades\",\"data\":{}}";

    const char *bad_levels =
        "{\"channel\":\"l2Book\",\"data\":{\"coin\":\"HYPE\",\"time\":1,\"levels\":[[]]}}";

    // ---- Case 1: happy path ----
    if (!push_msg(in, sample)) {
        std::printf("input buffer full (sample)\n");
        return 1;
    }

    parser.poll();

    std::size_t count1 = 0;
    event::l2_level_event ev{};
    while (out.pop(ev)) {
        ++count1;
        std::printf("sym=%s side=%u px=%lld sz=%lld n=%u t=%llu\n", ev.symbol.value,
                    static_cast<unsigned>(ev.level_side), static_cast<long long>(ev.price),
                    static_cast<long long>(ev.size), ev.order_count,
                    static_cast<unsigned long long>(ev.exchange_time_ms));
    }
    std::printf("case1_emitted=%zu\n", count1);

    if (count1 != 2) {
        std::printf("FAIL: expected 2 events for sample, got %zu\n", count1);
        return 2;
    }

    // ---- Case 2: wrong channel -> expect 0 ----
    if (!push_msg(in, wrong_channel)) {
        std::printf("input buffer full (wrong_channel)\n");
        return 3;
    }

    parser.poll();

    std::size_t count2 = 0;
    while (out.pop(ev)) {
        ++count2;
    }
    std::printf("case2_emitted=%zu\n", count2);

    if (count2 != 0) {
        std::printf("FAIL: expected 0 events for wrong channel, got %zu\n", count2);
        return 4;
    }

    // ---- Case 3: malformed levels -> expect 0 ----
    if (!push_msg(in, bad_levels)) {
        std::printf("input buffer full (bad_levels)\n");
        return 5;
    }

    parser.poll();

    std::size_t count3 = 0;
    while (out.pop(ev)) {
        ++count3;
    }
    std::printf("case3_emitted=%zu\n", count3);

    if (count3 != 0) {
        std::printf("FAIL: expected 0 events for malformed levels, got %zu\n", count3);
        return 6;
    }

    std::printf("PASS\n");
    return 0;
}
