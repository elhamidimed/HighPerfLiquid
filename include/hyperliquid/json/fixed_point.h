#pragma once

#include <cstddef>
#include <cstdint>

namespace hyperliquid::json {

// Parse ASCII decimal into int64 scaled by 1e6.
// Accepts forms like: "22", "22.4", "22.46", "22.460001"
// Extra decimals beyond 6 are truncated (not rounded) for determinism.
// Returns false on invalid input.
inline bool parse_decimal_1e6(const char *s, std::size_t n, std::int64_t &out) noexcept {
    if (s == nullptr || n == 0)
        return false;

    std::size_t i = 0;
    bool neg = false;
    if (s[i] == '-') {
        neg = true;
        ++i;
        if (i == n)
            return false;
    }

    std::int64_t int_part = 0;
    bool have_digits = false;

    // integer part
    for (; i < n; ++i) {
        const char c = s[i];
        if (c >= '0' && c <= '9') {
            have_digits = true;
            int_part = int_part * 10 + (c - '0');
        } else {
            break;
        }
    }

    std::int64_t frac_part = 0;
    std::int64_t scale = 1'000'000;

    if (i < n && s[i] == '.') {
        ++i;
        std::int64_t frac_scale = 100'000; // first digit is 1e5
        for (; i < n; ++i) {
            const char c = s[i];
            if (c >= '0' && c <= '9') {
                have_digits = true;
                if (frac_scale > 0) { // keep up to 6 decimals
                    frac_part += (c - '0') * frac_scale;
                    frac_scale /= 10;
                } else {
                    // truncate extra decimals deterministically
                }
            } else {
                return false; // invalid char after '.'
            }
        }
    } else {
        // if we broke on a non-digit and it's not '.', invalid
        if (i < n)
            return false;
    }

    if (!have_digits)
        return false;

    std::int64_t value = int_part * scale + frac_part;
    out = neg ? -value : value;
    return true;
}

} // namespace hyperliquid::json
