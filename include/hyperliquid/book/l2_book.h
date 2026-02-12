#pragma once

#include "hyperliquid/book/level.h"
#include "hyperliquid/market/types.h"

#include <array>
#include <cstddef>

namespace hyperliquid::book {

template <std::size_t Depth> class l2_book {
  public:
    static constexpr std::size_t depth = Depth;

    l2_book() noexcept = default;

    const hyperliquid::market::symbol_t &symbol() const noexcept {
        return symbol_;
    }
    hyperliquid::market::timestamp_ms_t last_update_time() const noexcept {
        return last_update_ms_;
    }

    const std::array<level, Depth> &bids() const noexcept {
        return bids_;
    }
    const std::array<level, Depth> &asks() const noexcept {
        return asks_;
    }

    std::size_t bid_count() const noexcept {
        return bid_count_;
    }
    std::size_t ask_count() const noexcept {
        return ask_count_;
    }

    // Clear book content (does not change symbol)
    void clear() noexcept {
        bid_count_ = 0;
        ask_count_ = 0;
        last_update_ms_ = 0;
    }

  private:
    hyperliquid::market::symbol_t symbol_{};
    hyperliquid::market::timestamp_ms_t last_update_ms_{0};

    std::array<level, Depth> bids_{};
    std::array<level, Depth> asks_{};

    std::size_t bid_count_{0};
    std::size_t ask_count_{0};
};

} // namespace hyperliquid::book