#pragma once

#include "hyperliquid/market/types.h"

#include <cstddef>
#include <cstring>
#include <string_view>

namespace hyperliquid::market {

// Returns false if input too long (>5)
inline bool set_symbol(symbol_t &dst, std::string_view s) noexcept {
    if (s.size() > 5)
        return false;
    std::memset(dst.value, 0, sizeof(dst.value));
    std::memcpy(dst.value, s.data(), s.size());
    dst.value[s.size()] = '\0';
    return true;
}

// Equality compare (fixed size, deterministic)
inline bool equals(const symbol_t &a, const symbol_t &b) noexcept {
    return std::memcmp(a.value, b.value, sizeof(a.value)) == 0;
}

// Compare symbol with literal / string_view (bounded)
inline bool equals(const symbol_t &a, std::string_view b) noexcept {
    // If b longer than 5 => cannot equal
    if (b.size() > 5)
        return false;
    // Compare only b.size(), and ensure null padding matches
    // Simpler: build a temporary symbol_t? (no, avoid)
    // Do deterministic check:
    if (std::memcmp(a.value, b.data(), b.size()) != 0)
        return false;
    // Remaining bytes must be '\0'
    for (std::size_t i = b.size(); i < 6; ++i) {
        if (a.value[i] != '\0')
            return false;
    }
    return true;
}

} // namespace hyperliquid::market
