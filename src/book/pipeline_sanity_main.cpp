#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/l2_book_parser.h"
#include "hyperliquid/market/symbol.h"

#include <cstdio>
#include <cstring>

int main() {
    using namespace hyperliquid;

    // Raw input from WS
    event::ring_buffer<event::market_event, 16> raw_in{};

    // Parsed L2 level updates
    event::ring_buffer<event::l2_level_event, 4096> l2_out{};

    // Parser stage
    json::l2_book_parser<16, 4096> parser(raw_in, l2_out);

    // Book + updater stage
    book::l2_book<20> bk;
    book::book_updater<4096, 20> updater(l2_out, bk);

    // A minimal snapshot: 1 bid + 1 ask
    const char *sample =
        "{\"channel\":\"l2Book\",\"data\":{\"coin\":\"HYPE\",\"time\":1769339484158,"
        "\"levels\":[[{\"px\":\"22.46\",\"sz\":\"265.68\",\"n\":5}],"
        "[{\"px\":\"22.461\",\"sz\":\"62.37\",\"n\":2}]]}}";

    event::market_event me{};
    me.size = std::strlen(sample);
    std::memcpy(me.data.data(), sample, me.size);

    if (!raw_in.push(me)) {
        std::printf("raw_in full\n");
        return 1;
    }

    parser.poll();  // raw_in -> l2_out
    updater.poll(); // l2_out -> bk

    // Validate book
    market::symbol_t hype{};
    market::set_symbol(hype, "HYPE");

    if (!market::equals(bk.symbol(), hype)) {
        std::printf("FAIL: symbol mismatch\n");
        return 2;
    }
    if (bk.last_update_time() != 1769339484158ULL) {
        std::printf("FAIL: time mismatch\n");
        return 3;
    }
    if (bk.bid_count() != 1 || bk.ask_count() != 1) {
        std::printf("FAIL: counts bid=%zu ask=%zu\n", bk.bid_count(), bk.ask_count());
        return 4;
    }
    if (bk.bids()[0].price != 22'460'000 || bk.asks()[0].price != 22'461'000) {
        std::printf("FAIL: prices bid=%lld ask=%lld\n", (long long)bk.bids()[0].price,
                    (long long)bk.asks()[0].price);
        return 5;
    }

    std::printf("PASS\n");
    return 0;
}