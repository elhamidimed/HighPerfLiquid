#pragma once

#include "hyperliquid/latency/clock.h"
#include "hyperliquid/latency/histogram.h"

#include <cstdint>
#include <cstdio>

namespace hyperliquid::bench {

struct run_config {
    int warmup_iters{1000};
    int measure_iters{10000};
};

inline double seconds_between(hyperliquid::latency::time_point a,
                              hyperliquid::latency::time_point b) noexcept {
    using namespace std::chrono;
    return duration_cast<duration<double>>(b - a).count();
}

template <typename Fn>
inline void run(const char *name, const run_config &cfg,
                hyperliquid::latency::log2_histogram<64> &hist, Fn &&fn) {
    hist.reset();

    // Warmup (not recorded)
    for (int i = 0; i < cfg.warmup_iters; ++i) {
        fn(false);
    }

    const auto t0 = hyperliquid::latency::now();
    for (int i = 0; i < cfg.measure_iters; ++i) {
        fn(true); // record
    }
    const auto t1 = hyperliquid::latency::now();

    const double secs = seconds_between(t0, t1);
    const double rate = (secs > 0.0) ? (static_cast<double>(cfg.measure_iters) / secs) : 0.0;

    std::printf("%s: iters=%d secs=%.6f rate=%.0f msg/s  "
                "p50=%lldns p99=%lldns p999=%lldns mean=%.1fns max=%lldns\n",
                name, cfg.measure_iters, secs, rate, (long long)hist.percentile_ns(0.50),
                (long long)hist.percentile_ns(0.99), (long long)hist.percentile_ns(0.999),
                hist.mean_ns(), (long long)hist.max_ns());
}

} // namespace hyperliquid::bench