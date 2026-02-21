#pragma once

#include "hyperliquid/market/types.h"

#include <cstdint>
#include <type_traits>

namespace hyperliquid::risk {

struct risk_limits {
    // Max absolute qty per order
    hyperliquid::market::qty_t max_order_qty{0};

    // Max notional per order (price*qty scaled)
    // represent as 1e6-scaled
    hyperliquid::market::price_t max_order_notional{0};

    std::uint8_t require_post_only{0};
    std::uint8_t allow_market_orders{1};
};

static_assert(std::is_trivially_copyable_v<risk_limits>);
static_assert(std::is_standard_layout_v<risk_limits>);

} // namespace hyperliquid::risk