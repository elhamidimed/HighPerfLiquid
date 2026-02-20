#pragma once

#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/trading/order_events.h"

#include <cstddef>

namespace hyperliquid::json {

template <std::size_t InN, std::size_t OutN> class user_fills_parser {
  public:
    using in_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::event::market_event, InN>;
    using out_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::trading::order_event, OutN>;

    user_fills_parser(in_buffer_t &in, out_buffer_t &out) noexcept;

    void poll() noexcept;

  private:
    in_buffer_t &in_;
    out_buffer_t &out_;
};

} // namespace hyperliquid::json