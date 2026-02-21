#pragma once

#include <cstdint>
#include <string_view>

namespace hyperliquid::config {

struct risk_config {
    // Same units the trading model (scaled 1e6).
    std::int64_t max_order_qty_1e6 = 0;      // 0 = disabled
    std::int64_t max_order_notional_1e6 = 0; // 0 = disabled

    bool require_post_only = false;
    bool allow_market_orders = true;
};

struct library_config {
    std::string_view log_level = "info";
    std::uint32_t network_timeout_ms = 500;

    risk_config risk{};
};

} // namespace hyperliquid::config