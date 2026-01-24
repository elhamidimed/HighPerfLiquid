#include "hyperliquid/core/timestamp.h"

#include <chrono>

namespace hyperliquid::core {

nanoseconds now_ns() noexcept {
    return static_cast<nanoseconds>(std::chrono::steady_clock::now().time_since_epoch().count());
}

} // namespace hyperliquid::core
