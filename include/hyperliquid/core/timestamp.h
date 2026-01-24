#pragma once

#include <cstdint>

namespace hyperliquid::core {

using nanoseconds = std::uint64_t;

nanoseconds now_ns() noexcept;

} // namespace hyperliquid::core
