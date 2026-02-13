#pragma once

#include "hyperliquid/latency/histogram.h"
#include "hyperliquid/latency/instrument.h"

namespace hyperliquid::latency {

template <typename Stage, std::size_t Buckets = 64> class timed_stage {
  public:
    explicit timed_stage(Stage &s) noexcept : stage_(s) {}

    void poll() noexcept {
        scoped<log2_histogram<Buckets>> t(hist_);
        stage_.poll();
    }

    log2_histogram<Buckets> &histogram() noexcept {
        return hist_;
    }
    const log2_histogram<Buckets> &histogram() const noexcept {
        return hist_;
    }

  private:
    Stage &stage_;
    log2_histogram<Buckets> hist_{};
};

} // namespace hyperliquid::latency