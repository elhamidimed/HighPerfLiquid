#pragma once

#include <cstdint>
#include <type_traits>

namespace hyperliquid::market {

inline constexpr std::int64_t k_fixed_scale = 1'000'000;

struct symbol_t {
    char value[6]; // 5 + '\0'
};

using price_t = std::int64_t;
using qty_t = std::int64_t;
using timestamp_ms_t = std::uint64_t;

static_assert(sizeof(symbol_t) == 6, "symbol_t must be 6 bytes");
static_assert(std::is_trivially_copyable_v<symbol_t>, "symbol_t must be trivially copyable");
static_assert(std::is_standard_layout_v<symbol_t>, "symbol_t must be standard layout");

} // namespace hyperliquid::market
