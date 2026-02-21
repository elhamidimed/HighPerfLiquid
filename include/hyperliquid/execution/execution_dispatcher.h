#pragma once

#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/padded_msg.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

#include <simdjson.h>

namespace hyperliquid::execution {

// Routes raw WS messages by channel into dedicated parser input queues.
// The reason is to prevents multiple parsers competing for the same raw input ring.
template <std::size_t RawInN, std::size_t UpdatesInN, std::size_t FillsInN>
class execution_dispatcher {
  public:
    using raw_in_t = hyperliquid::event::ring_buffer<hyperliquid::event::market_event, RawInN>;
    using updates_in_t =
        hyperliquid::event::ring_buffer<hyperliquid::event::market_event, UpdatesInN>;
    using fills_in_t = hyperliquid::event::ring_buffer<hyperliquid::event::market_event, FillsInN>;

    execution_dispatcher(raw_in_t &raw, updates_in_t &upd, fills_in_t &fills) noexcept
        : raw_(raw), updates_(upd), fills_(fills) {}

    void poll() noexcept {
        simdjson::ondemand::parser p;
        hyperliquid::event::market_event me{};

        while (raw_.pop(me)) {
            hyperliquid::json::padded_msg<4096> msg{};
            if (!msg.load(me.data.data(), me.size)) {
                continue;
            }

            auto doc_res = p.iterate(msg.data(), msg.size(), msg.capacity());
            if (doc_res.error())
                continue;

            simdjson::ondemand::document &doc = doc_res.value_unsafe();
            auto ch = doc["channel"].get_string();
            if (ch.error())
                continue;

            const std::string_view channel = ch.value_unsafe();

            if (channel == "orderUpdates") {
                (void)updates_.push(me); // ignoring the return for now (drop if full)
            } else if (channel == "userFills") {
                (void)fills_.push(me);
            } else {
                // I'll ignore for now
            }
        }
    }

  private:
    raw_in_t &raw_;
    updates_in_t &updates_;
    fills_in_t &fills_;
};

} // namespace hyperliquid::execution