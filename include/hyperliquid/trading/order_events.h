#pragma once

#include "hyperliquid/market/types.h"
#include "hyperliquid/trading/types.h"

#include <cstdint>
#include <type_traits>

namespace hyperliquid::trading {

// venue order id assigned by exchange.
struct venue_order_id {
    char value[32];
};

static_assert(sizeof(venue_order_id) == 32);
static_assert(std::is_trivially_copyable_v<venue_order_id>);
static_assert(std::is_standard_layout_v<venue_order_id>);

enum class order_event_type : std::uint8_t { ack = 0, reject = 1, fill = 2, cancel_ack = 3 };

// Generic reason code to extend
enum class reject_reason : std::uint8_t { unknown = 0, invalid = 1, risk = 2, venue = 3 };

struct order_ack {
    client_order_id client_id;
    venue_order_id venue_id;
    hyperliquid::market::timestamp_ms_t exchange_time_ms;
};

struct order_reject {
    client_order_id client_id;
    reject_reason reason;
    // optional
    char message[64];
    hyperliquid::market::timestamp_ms_t exchange_time_ms;
};

struct order_fill {
    client_order_id client_id;
    venue_order_id venue_id;

    side s;

    // Execution
    hyperliquid::market::price_t fill_price;
    hyperliquid::market::qty_t fill_qty;

    // Optional cumulative info
    hyperliquid::market::qty_t cum_qty;

    hyperliquid::market::timestamp_ms_t exchange_time_ms;
};

struct order_cancel_ack {
    client_order_id client_id;
    venue_order_id venue_id;
    hyperliquid::market::timestamp_ms_t exchange_time_ms;
};

// A tagged union for downstream handling (no std::variant; deterministic layout)
struct order_event {
    order_event_type type;
    union {
        order_ack ack;
        order_reject reject;
        order_fill fill;
        order_cancel_ack cancel;
    };
};

static_assert(std::is_trivially_copyable_v<order_ack>);
static_assert(std::is_trivially_copyable_v<order_reject>);
static_assert(std::is_trivially_copyable_v<order_fill>);
static_assert(std::is_trivially_copyable_v<order_cancel_ack>);
static_assert(std::is_trivially_copyable_v<order_event>);
static_assert(std::is_standard_layout_v<order_event>);

} // namespace hyperliquid::trading