#include "hyperliquid/latency/histogram.h"
#include "hyperliquid/latency/scope.h"

#include <cstdio>

int main() {
    using namespace hyperliquid::latency;

    log2_histogram<64> h;
    h.reset();

    // Recording some synthetic durations
    for (int i = 0; i < 1000; ++i) {
        scope_timer<log2_histogram<64>> t(h);
        // do nothing (measures tiny scope)
    }

    std::printf("count=%llu min=%lldns mean=%.1fns max=%lldns p50=%lldns p99=%lldns p999=%lldns\n",
                (unsigned long long)h.count(), (long long)h.min_ns(), h.mean_ns(),
                (long long)h.max_ns(), (long long)h.percentile_ns(0.50),
                (long long)h.percentile_ns(0.99), (long long)h.percentile_ns(0.999));

    return (h.count() > 0) ? 0 : 1;
}