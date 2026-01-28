#pragma once

#include "hyperliquid/market/types.h"

#include <type_traits>

namespace hyperliquid::event {

enum class side : std::uint8_t { bid = 0, ask = 1 };

struct l2_level_event {
    hyperliquid::market::symbol_t symbol;
    side level_side;
    hyperliquid::market::price_t price;
    hyperliquid::market::qty_t size;
    uint32_t order_count;
    hyperliquid::market::timestamp_ms_t exchange_time_ms;
};

static_assert(std::is_trivially_copyable_v<hyperliquid::event::l2_level_event>,
              "l2_level_event must be trivially copyable");
static_assert(std::is_standard_layout_v<hyperliquid::event::l2_level_event>,
              "l2_level_event must be standard layout");

} // namespace hyperliquid::event
