#pragma once

#include "hyperliquid/trading/order_request.h"

#include <cstdint>

namespace hyperliquid::trading {

enum class validate_error : std::uint8_t {
    ok = 0,
    qty_non_positive = 1,
    limit_price_non_positive = 2,
    market_price_must_be_zero = 3
};

inline validate_error validate(const order_request &r) noexcept {
    if (r.qty <= 0)
        return validate_error::qty_non_positive;

    if (r.type == order_type::limit) {
        if (r.price <= 0)
            return validate_error::limit_price_non_positive;
    } else { // market
        if (r.price != 0)
            return validate_error::market_price_must_be_zero;
    }

    return validate_error::ok;
}

} // namespace hyperliquid::trading