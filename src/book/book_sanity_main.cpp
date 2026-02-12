#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/book/l2_snapshot_builder.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/market/symbol.h"

#include <cstdio>

int main() {
    using namespace hyperliquid;

    book::l2_book<20> bk;
    book::l2_snapshot_builder<20> builder(bk);

    event::l2_level_event e1{};
    market::set_symbol(e1.symbol, "HYPE");
    e1.exchange_time_ms = 100;
    e1.level_side = event::side::bid;
    e1.price = 10'000'000;
    e1.size = 1'000'000;
    e1.order_count = 1;

    event::l2_level_event e2 = e1;
    e2.level_side = event::side::ask;
    e2.price = 11'000'000;

    builder.on_level(e1);
    builder.on_level(e2);
    builder.flush();

    if (!market::equals(bk.symbol(), e1.symbol))
        return 1;
    if (bk.last_update_time() != 100)
        return 2;
    if (bk.bid_count() != 1 || bk.ask_count() != 1)
        return 3;
    if (bk.bids()[0].price != 10'000'000)
        return 4;
    if (bk.asks()[0].price != 11'000'000)
        return 5;

    std::printf("PASS\n");
    return 0;
}