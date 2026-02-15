#include "hyperliquid/bench/bench.h"
#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/latency/histogram.h"
#include "hyperliquid/latency/scope.h"
#include "hyperliquid/market/symbol.h"

#include <cstdio>

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::l2_level_event, 8192> in{};
    book::l2_book<20> bk;
    book::book_updater<8192, 20> updater(in, bk);

    // Pre-generate one snapshot batch of 40 events
    event::l2_level_event batch[40]{};
    for (int i = 0; i < 40; ++i) {
        market::set_symbol(batch[i].symbol, "HYPE");
        batch[i].exchange_time_ms = 123;
        batch[i].level_side = (i < 20) ? event::side::bid : event::side::ask;
        if (i < 20) {
            batch[i].price = 30'000'000 - i * 1'000;
        } else {
            batch[i].price = 30'000'000 + (i - 20) * 1'000;
        }
        batch[i].size = 1'000'000;
        batch[i].order_count = 1;
    }

    latency::log2_histogram<64> hist;
    bench::run_config cfg{};
    cfg.warmup_iters = 5000;
    cfg.measure_iters = 50000;

    bench::run("bench_updater.poll", cfg, hist, [&](bool record) {
        // push 40 events
        for (int i = 0; i < 40; ++i) {
            in.push(batch[i]);
        }

        if (record) {
            latency::scope_timer<latency::log2_histogram<64>> t(hist);
            updater.poll();
        } else {
            updater.poll();
        }
    });

    return 0;
}