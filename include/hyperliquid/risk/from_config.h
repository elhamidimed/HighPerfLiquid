#pragma once

#include "hyperliquid/config/config.h"
#include "hyperliquid/risk/risk_limits.h"

namespace hyperliquid::risk {

inline risk_limits from_config(const hyperliquid::config::library_config &cfg) noexcept {
    risk_limits out{};

    out.max_order_qty = static_cast<hyperliquid::market::qty_t>(cfg.risk.max_order_qty_1e6);
    out.max_order_notional =
        static_cast<hyperliquid::market::price_t>(cfg.risk.max_order_notional_1e6);

    out.require_post_only = cfg.risk.require_post_only ? 1 : 0;
    out.allow_market_orders = cfg.risk.allow_market_orders ? 1 : 0;

    return out;
}

} // namespace hyperliquid::risk