#pragma once

#include "hyperliquid/latency/histogram.h"
#include "hyperliquid/latency/scope.h"

namespace hyperliquid::latency {

#ifdef HPL_ENABLE_LATENCY

template <typename Histogram> using scoped = scope_timer<Histogram>;

#else

// When disabled, scoped does nothing and compiles away.
template <typename Histogram> struct scoped {
    explicit scoped(Histogram &) noexcept {}
};

#endif

} // namespace hyperliquid::latency