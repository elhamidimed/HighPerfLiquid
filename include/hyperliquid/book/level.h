#pragma once

#include "hyperliquid/market/types.h"

#include <cstdint>

namespace hyperliquid::book {

struct level {
    hyperliquid::market::price_t price;
    hyperliquid::market::qty_t size;
    std::uint32_t order_count;
};

} // namespace hyperliquid::book