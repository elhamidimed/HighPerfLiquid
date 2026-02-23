#include "hyperliquid/ws/wss_client.h"

#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"

#include "hyperliquid/json/l2_book_parser.h"

#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"

#include <cstdio>
#include <cstring>
#include <string_view>

int main() {
    using namespace hyperliquid;

    std::uint64_t rx_text = 0;
    std::uint64_t dropped_oversize = 0;
    std::uint64_t dropped_raw_full = 0;
    std::uint64_t skipped_non_l2book = 0;
    std::uint64_t snapshots_printed = 0;

    // --- WS connect ---
    ws::wss_client c;
    if (!c.connect("api.hyperliquid.xyz", "/ws", 443)) {
        std::printf("FAIL connect/handshake\n");
        return 1;
    }

    const char *sub =
        R"json({"method":"subscribe","subscription":{"type":"l2Book","coin":"HYPE"}})json";
    if (!c.send_text(sub)) {
        std::printf("FAIL subscribe send\n");
        return 2;
    }

    // --- Pipeline wiring for demo ---
    event::ring_buffer<event::market_event, 256> raw_in{};
    event::ring_buffer<event::l2_level_event, 65536> parsed_out{};

    json::l2_book_parser<256, 65536> parser(raw_in, parsed_out);

    book::l2_book<20> bk{};
    book::book_updater<65536, 20> upd(parsed_out, bk);

    std::size_t snapshots = 0;
    constexpr std::size_t MAX_SNAPSHOTS = 50;

    ws::text_view tv{};
    while (snapshots < MAX_SNAPSHOTS) {
        auto st = c.poll(tv);
        if (st == ws::ws_status::error || st == ws::ws_status::closed) {
            std::printf("FAIL ws\n");
            return 3;
        }
        if (st != ws::ws_status::ok) {
            continue;
        }

        ++rx_text;

        // cheap channel filter
        const std::string_view sv(tv.data, tv.size);
        if (sv.find("\"channel\":\"l2Book\"") == std::string_view::npos) {
            ++skipped_non_l2book;
            continue;
        }

        event::market_event me{};
        if (tv.size > me.data.size()) {
            ++dropped_oversize;
            continue;
        }
        me.size = tv.size;
        std::memcpy(me.data.data(), tv.data, tv.size);

        // Push raw message
        if (!raw_in.push(me)) {
            continue;
        }

        parser.poll();
        upd.poll();
        if (bk.bid_count() > 0 && bk.ask_count() > 0) {
            const auto bb = bk.bids();
            const auto ba = bk.asks();

            std::printf("t=%llu bid=%lld x %lld | ask=%lld x %lld\n",
                        (unsigned long long)bk.last_update_time(), (long long)bb[0].price,
                        (long long)bb[0].size, (long long)ba[0].price, (long long)ba[0].size);

            ++snapshots;
            ++snapshots_printed;
        }
    }

    std::printf("summary: rx_text=%llu skipped_non_l2book=%llu dropped_oversize=%llu "
                "dropped_full=%llu printed=%llu\n",
                (unsigned long long)rx_text, (unsigned long long)skipped_non_l2book,
                (unsigned long long)dropped_oversize, (unsigned long long)dropped_raw_full,
                (unsigned long long)snapshots_printed);

    std::printf("PASS\n");
    return 0;
}