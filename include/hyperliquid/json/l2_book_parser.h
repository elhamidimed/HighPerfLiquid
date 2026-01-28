#pragma once

#include "hyperliquid/event/event.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/event/ring_buffer.h"

namespace hyperliquid::json {

template <std::size_t InN, std::size_t OutN> class l2_book_parser {
  public:
    using in_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::event::market_event, InN>;

    using out_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::event::l2_level_event, OutN>;

    l2_book_parser(in_buffer_t &in, out_buffer_t &out) noexcept : in_(in), out_(out) {}

    // Non-blocking, consumes all available input
    void poll() noexcept;

  private:
    in_buffer_t &in_;
    out_buffer_t &out_;
};

} // namespace hyperliquid::json
