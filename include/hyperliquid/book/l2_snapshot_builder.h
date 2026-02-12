#pragma once

#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/market/symbol.h"

#include <cstddef>
#include <cstring>

namespace hyperliquid::book {

// Collects l2_level_event stream into full snapshots and applies to an l2_book.
// Boundary rule: (symbol, exchange_time_ms) change => flush previous snapshot (if any).
template <std::size_t Depth> class l2_snapshot_builder {
  public:
    explicit l2_snapshot_builder(l2_book<Depth> &book) noexcept : book_(book) {}

    void on_level(const hyperliquid::event::l2_level_event &ev) noexcept {
        const auto ev_time = ev.exchange_time_ms;

        if (!have_key_) {
            begin_new(ev);
        } else if (ev_time != cur_time_ || !hyperliquid::market::equals(cur_symbol_, ev.symbol)) {
            apply_to_book();
            begin_new(ev);
        }

        if (ev.level_side == hyperliquid::event::side::bid) {
            if (bid_count_ < Depth) {
                bids_[bid_count_++] = level_from(ev);
            }
        } else {
            if (ask_count_ < Depth) {
                asks_[ask_count_++] = level_from(ev);
            }
        }
    }

    void flush() noexcept {
        if (have_key_) {
            apply_to_book();
            have_key_ = false;
        }
    }

  private:
    static level level_from(const hyperliquid::event::l2_level_event &ev) noexcept {
        return level{ev.price, ev.size, ev.order_count};
    }

    void begin_new(const hyperliquid::event::l2_level_event &ev) noexcept {
        cur_symbol_ = ev.symbol;
        cur_time_ = ev.exchange_time_ms;
        bid_count_ = 0;
        ask_count_ = 0;
        have_key_ = true;
    }

    void apply_to_book() noexcept {
        book_.apply_snapshot(cur_symbol_, cur_time_, bids_, bid_count_, asks_, ask_count_);
    }

    l2_book<Depth> &book_;

    bool have_key_{false};
    hyperliquid::market::symbol_t cur_symbol_{};
    hyperliquid::market::timestamp_ms_t cur_time_{0};

    std::array<level, Depth> bids_{};
    std::array<level, Depth> asks_{};
    std::size_t bid_count_{0};
    std::size_t ask_count_{0};
};

} // namespace hyperliquid::book