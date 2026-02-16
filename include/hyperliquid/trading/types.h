#pragma once

#include <cstdint>
#include <type_traits>

namespace hyperliquid::trading {

enum class side : std::uint8_t { buy = 0, sell = 1 };

enum class order_type : std::uint8_t { limit = 0, market = 1 };

enum class time_in_force : std::uint8_t { gtc = 0, ioc = 1, fok = 2 };

// Fixed-size client order id (ASCII), 31 + null.
struct client_order_id {
    char value[32];
};

static_assert(sizeof(client_order_id) == 32);
static_assert(std::is_trivially_copyable_v<client_order_id>);
static_assert(std::is_standard_layout_v<client_order_id>);

} // namespace hyperliquid::trading