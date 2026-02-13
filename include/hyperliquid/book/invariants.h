#pragma once

#include "hyperliquid/book/level.h"

#include <cstddef>
#include <cstdint>

namespace hyperliquid::book {

// Return true if bids are sorted by price descending (non-increasing).
inline bool bids_sorted_desc(const level *lvls, std::size_t n) noexcept {
    if (n <= 1)
        return true;
    for (std::size_t i = 1; i < n; ++i) {
        if (lvls[i - 1].price < lvls[i].price)
            return false;
    }
    return true;
}

// Return true if asks are sorted by price ascending (non-decreasing).
inline bool asks_sorted_asc(const level *lvls, std::size_t n) noexcept {
    if (n <= 1)
        return true;
    for (std::size_t i = 1; i < n; ++i) {
        if (lvls[i - 1].price > lvls[i].price)
            return false;
    }
    return true;
}

inline bool sizes_non_negative(const level *lvls, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        if (lvls[i].size < 0)
            return false;
    }
    return true;
}

} // namespace hyperliquid::book