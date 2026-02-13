#include "hyperliquid/book/book_updater.h"
#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/l2_book_parser.h"
#include "hyperliquid/latency/stage_wrapper.h"

#include <cstdio>
#include <cstring>

static void print_hist(const char *name, const hyperliquid::latency::log2_histogram<64> &h) {
    std::printf(
        "%s: count=%llu min=%lldns mean=%.1fns max=%lldns p50=%lldns p99=%lldns p999=%lldns\n",
        name, (unsigned long long)h.count(), (long long)h.min_ns(), h.mean_ns(),
        (long long)h.max_ns(), (long long)h.percentile_ns(0.50), (long long)h.percentile_ns(0.99),
        (long long)h.percentile_ns(0.999));
}

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 16> raw_in{};
    event::ring_buffer<event::l2_level_event, 4096> l2_out{};

    json::l2_book_parser<16, 4096> parser(raw_in, l2_out);
    book::l2_book<20> bk;
    book::book_updater<4096, 20> updater(l2_out, bk);

    latency::timed_stage<decltype(parser)> timed_parser(parser);
    latency::timed_stage<decltype(updater)> timed_updater(updater);

    const char *sample =
        "{\"channel\":\"l2Book\",\"data\":{\"coin\":\"HYPE\",\"time\":1769339484158,"
        "\"levels\":[[{\"px\":\"22.46\",\"sz\":\"265.68\",\"n\":5}],"
        "[{\"px\":\"22.461\",\"sz\":\"62.37\",\"n\":2}]]}}";

    // Push the same message many times to get a distribution
    for (int i = 0; i < 10000; ++i) {
        event::market_event me{};
        me.size = std::strlen(sample);
        std::memcpy(me.data.data(), sample, me.size);
        raw_in.push(me);

        timed_parser.poll();
        timed_updater.poll();
    }

#ifdef HPL_ENABLE_LATENCY
    print_hist("parser.poll", timed_parser.histogram());
    print_hist("updater.poll", timed_updater.histogram());
#else
    std::printf("HPL_ENABLE_LATENCY not enabled (no timing collected)\n");
#endif

    return 0;
}