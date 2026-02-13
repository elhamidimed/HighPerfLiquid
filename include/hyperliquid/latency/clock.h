#pragma once

#include <chrono>
#include <cstdint>

namespace hyperliquid::latency {

using clock = std::chrono::steady_clock;
using time_point = clock::time_point;

// Nanoseconds
inline std::int64_t ns_since(time_point start, time_point end) noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

inline time_point now() noexcept {
    return clock::now();
}

} // namespace hyperliquid::latency