#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/l2_book_parser.h"
#include "hyperliquid/market/symbol.h"

#include <cstdio>
#include <cstring>

namespace {
int fail(const char *msg, int code) {
    std::printf("FAIL(%d): %s\n", code, msg);
    return code;
}
} // namespace

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 16> raw_in{};
    event::ring_buffer<event::l2_level_event, 4096> l2_out{};

    json::l2_book_parser<16, 4096> parser(raw_in, l2_out);

    book::l2_book<20> bk;
    book::book_updater<4096, 20> updater(l2_out, bk);

    const char *sample =
        "{\"channel\":\"l2Book\",\"data\":{\"coin\":\"HYPE\",\"time\":1769339484158,"
        "\"levels\":[[{\"px\":\"22.46\",\"sz\":\"265.68\",\"n\":5}],"
        "[{\"px\":\"22.461\",\"sz\":\"62.37\",\"n\":2}]]}}";

    event::market_event me{};
    me.size = std::strlen(sample);
    std::memcpy(me.data.data(), sample, me.size);

    if (!raw_in.push(me))
        return fail("raw_in full", 1);

    parser.poll();
    updater.poll();

    market::symbol_t hype{};
    market::set_symbol(hype, "HYPE");

    if (!market::equals(bk.symbol(), hype))
        return fail("symbol mismatch", 2);
    if (bk.last_update_time() != 1769339484158ULL)
        return fail("time mismatch", 3);
    if (bk.bid_count() != 1 || bk.ask_count() != 1)
        return fail("counts mismatch", 4);

    if (bk.bids()[0].price != 22'460'000)
        return fail("bid price mismatch", 5);
    if (bk.asks()[0].price != 22'461'000)
        return fail("ask price mismatch", 6);

    std::printf("PASS\n");
    return 0;
}