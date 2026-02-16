#pragma once

#include "hyperliquid/market/types.h"
#include "hyperliquid/trading/types.h"

#include <cstdint>
#include <type_traits>

namespace hyperliquid::trading {

struct order_request {
    hyperliquid::market::symbol_t symbol;

    side s;
    order_type type;
    time_in_force tif;

    // Flags (kept as bytes for predictable layout)
    std::uint8_t post_only;
    std::uint8_t reduce_only;

    hyperliquid::market::price_t price; // only used for limit
    hyperliquid::market::qty_t qty;

    client_order_id client_id;
};

static_assert(std::is_trivially_copyable_v<order_request>);
static_assert(std::is_standard_layout_v<order_request>);

} // namespace hyperliquid::trading