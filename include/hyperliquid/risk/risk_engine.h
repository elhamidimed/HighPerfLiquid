#pragma once

#include "hyperliquid/risk/risk_limits.h"
#include "hyperliquid/trading/order_request.h"
#include "hyperliquid/trading/validate.h"

#include <cstdint>

namespace hyperliquid::risk {

enum class decision_reason : std::uint8_t {
    ok = 0,
    invalid_request = 1,
    post_only_required = 2,
    market_not_allowed = 3,
    qty_limit = 4,
    notional_limit = 5
};

struct decision {
    std::uint8_t allow;
    decision_reason reason;
};

class risk_engine {
  public:
    explicit risk_engine(const risk_limits &lim) noexcept : lim_(lim) {}

    decision check(const hyperliquid::trading::order_request &r) const noexcept {
        // structural validation
        if (hyperliquid::trading::validate(r) != hyperliquid::trading::validate_error::ok) {
            return {0, decision_reason::invalid_request};
        }

        if (lim_.require_post_only && !r.post_only) {
            return {0, decision_reason::post_only_required};
        }

        if (!lim_.allow_market_orders && r.type == hyperliquid::trading::order_type::market) {
            return {0, decision_reason::market_not_allowed};
        }

        if (lim_.max_order_qty > 0 && r.qty > lim_.max_order_qty) {
            return {0, decision_reason::qty_limit};
        }

        // Notional = price * qty / 1e6  (all are 1e6 scaled)
        if (lim_.max_order_notional > 0 && r.type == hyperliquid::trading::order_type::limit) {
            // using 128-bit to avoid overflow
            __extension__ __int128 prod =
                static_cast<__int128>(r.price) * static_cast<__int128>(r.qty);
            __extension__ __int128 notional =
                prod / static_cast<__int128>(hyperliquid::market::k_fixed_scale);
            if (notional > static_cast<__int128>(lim_.max_order_notional)) {
                return {0, decision_reason::notional_limit};
            }
        }

        return {1, decision_reason::ok};
    }

  private:
    risk_limits lim_;
};

} // namespace hyperliquid::risk