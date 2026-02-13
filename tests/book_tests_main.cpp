#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/book/l2_snapshot_builder.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/market/symbol.h"

#include <cstdio>

namespace {

using namespace hyperliquid;

int fail(const char *msg, int code) {
    std::printf("FAIL(%d): %s\n", code, msg);
    return code;
}

event::l2_level_event make_ev(const char *sym, std::uint64_t t, event::side s, market::price_t px,
                              market::qty_t sz, std::uint32_t n) {
    event::l2_level_event ev{};
    market::set_symbol(ev.symbol, sym);
    ev.exchange_time_ms = t;
    ev.level_side = s;
    ev.price = px;
    ev.size = sz;
    ev.order_count = n;
    return ev;
}

} // namespace

int main() {
    // ---- Test 1: snapshot replacement ----
    {
        book::l2_book<20> bk;
        book::l2_snapshot_builder<20> b(bk);

        // Snapshot time 100
        b.on_level(make_ev("HYPE", 100, event::side::bid, 10'000'000, 1'000'000, 1));
        b.on_level(make_ev("HYPE", 100, event::side::ask, 11'000'000, 2'000'000, 1));
        b.flush();

        if (bk.last_update_time() != 100)
            return fail("snapshot A time", 1);
        if (bk.bid_count() != 1 || bk.ask_count() != 1)
            return fail("snapshot A counts", 2);
        if (bk.bids()[0].price != 10'000'000)
            return fail("snapshot A bid px", 3);

        // Snapshot time 200 replaces
        b.on_level(make_ev("HYPE", 200, event::side::bid, 9'000'000, 3'000'000, 1));
        b.on_level(make_ev("HYPE", 200, event::side::ask, 12'000'000, 4'000'000, 1));
        b.flush();

        if (bk.last_update_time() != 200)
            return fail("snapshot B time", 4);
        if (bk.bids()[0].price != 9'000'000)
            return fail("snapshot B bid px", 5);
        if (bk.asks()[0].price != 12'000'000)
            return fail("snapshot B ask px", 6);
    }

    // ---- Test 2: partial depth ----
    {
        book::l2_book<20> bk;
        book::l2_snapshot_builder<20> b(bk);

        // only 2 bids, 1 ask
        b.on_level(make_ev("HYPE", 300, event::side::bid, 10'000'000, 1'000'000, 1));
        b.on_level(make_ev("HYPE", 300, event::side::bid, 9'000'000, 1'000'000, 1));
        b.on_level(make_ev("HYPE", 300, event::side::ask, 11'000'000, 1'000'000, 1));
        b.flush();

        if (bk.bid_count() != 2)
            return fail("partial depth bid_count", 7);
        if (bk.ask_count() != 1)
            return fail("partial depth ask_count", 8);
        if (bk.bids()[1].price != 9'000'000)
            return fail("partial depth bid[1]", 9);
    }

    // ---- Test 3: symbol boundary starts new snapshot ----
    {
        book::l2_book<20> bk;
        book::l2_snapshot_builder<20> b(bk);

        b.on_level(make_ev("HYPE", 400, event::side::bid, 10'000'000, 1'000'000, 1));
        // new symbol triggers apply of previous snapshot
        b.on_level(make_ev("BTC", 400, event::side::bid, 20'000'000, 1'000'000, 1));
        b.flush();

        market::symbol_t btc{};
        market::set_symbol(btc, "BTC");
        if (!market::equals(bk.symbol(), btc))
            return fail("symbol boundary", 10);
        if (bk.bids()[0].price != 20'000'000)
            return fail("symbol boundary bid px", 11);
    }

    std::printf("PASS\n");
    return 0;
}