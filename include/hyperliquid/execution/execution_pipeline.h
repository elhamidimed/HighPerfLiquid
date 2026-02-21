#pragma once

#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/order_updates_parser.h"
#include "hyperliquid/json/user_fills_parser.h"
#include "hyperliquid/trading/order_events.h"

#include <cstddef>

namespace hyperliquid::execution {

template <std::size_t InN, std::size_t OutN> class execution_pipeline {
  public:
    using in_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::event::market_event, InN>;
    using out_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::trading::order_event, OutN>;

    execution_pipeline(in_buffer_t &in, out_buffer_t &out) noexcept
        : in_(in), out_(out), order_updates_(in_, out_), user_fills_(in_, out_) {}

    // We poll all execution parsers on the same input stream
    void poll() noexcept {
        order_updates_.poll();
        user_fills_.poll();
    }

  private:
    in_buffer_t &in_;
    out_buffer_t &out_;
    hyperliquid::json::order_updates_parser<InN, OutN> order_updates_;
    hyperliquid::json::user_fills_parser<InN, OutN> user_fills_;
};

} // namespace hyperliquid::execution