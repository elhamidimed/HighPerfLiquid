#pragma once

#include "hyperliquid/market/types.h"

#include <cstddef>
#include <cstdint>

namespace hyperliquid::market {

// Example: 22460000 -> "22.46", 265680000 -> "265.68", 500000000 -> "500"
// Returns number of chars written, or 0 on failure (cap too small).
inline std::size_t to_decimal_1e6(price_t v, char *out, std::size_t cap) noexcept {
    // Need room for at least "0"
    if (cap == 0)
        return 0;

    std::uint64_t u;
    std::size_t pos = 0;

    if (v < 0) {
        if (pos + 1 > cap)
            return 0;
        out[pos++] = '-';
        u = static_cast<std::uint64_t>(-(v + 1)) + 1;
    } else {
        u = static_cast<std::uint64_t>(v);
    }

    const std::uint64_t int_part = u / static_cast<std::uint64_t>(k_fixed_scale);
    std::uint64_t frac = u % static_cast<std::uint64_t>(k_fixed_scale); // 0..999999

    // write int part into temp reversed
    char tmp[32];
    std::size_t n = 0;
    std::uint64_t x = int_part;
    do {
        tmp[n++] = char('0' + (x % 10));
        x /= 10;
    } while (x);

    if (pos + n > cap)
        return 0;
    for (std::size_t i = 0; i < n; ++i)
        out[pos + i] = tmp[n - 1 - i];
    pos += n;

    // If fractional is zero => done
    if (frac == 0) {
        if (pos >= cap)
            return 0;
        out[pos] = '\0';
        return pos;
    }

    // write '.'
    if (pos + 1 > cap)
        return 0;
    out[pos++] = '.';

    // write fractional as 6 digits, then trim trailing zeros
    char fracbuf[6];
    for (int i = 5; i >= 0; --i) {
        fracbuf[i] = char('0' + (frac % 10));
        frac /= 10;
    }

    int end = 5;
    while (end >= 0 && fracbuf[end] == '0')
        --end;

    const std::size_t frac_len = static_cast<std::size_t>(end + 1);
    if (pos + frac_len > cap)
        return 0;

    for (std::size_t i = 0; i < frac_len; ++i)
        out[pos + i] = fracbuf[i];
    pos += frac_len;

    if (pos >= cap)
        return 0;
    out[pos] = '\0';
    return pos;
}

} // namespace hyperliquid::market