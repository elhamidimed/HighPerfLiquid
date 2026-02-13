#pragma once

#include <cstdint>

namespace hyperliquid::latency {

// Minimal sink

struct null_sink {
    void record_ns(std::int64_t) noexcept {}
};

} // namespace hyperliquid::latency