#pragma once

#include <array>
#include <cstddef>
#include <cstring>

#include <simdjson.h>

namespace hyperliquid::json {

// Fixed buffer for simdjson: input + required padding.
// (No heap; per-message stack copy.)

template <std::size_t Max> struct padded_msg {
    char buf[Max + simdjson::SIMDJSON_PADDING];
    size_t len{0};

    bool load(const char *src, size_t n) noexcept {
        if (n > Max)
            return false;
        len = n;
        std::memcpy(buf, src, len);
        std::memset(buf + len, 0, simdjson::SIMDJSON_PADDING);
        return true;
    }

    simdjson::padded_string_view view() const noexcept {
        return simdjson::padded_string_view(buf, len);
    }

    constexpr size_t capacity() const noexcept {
        return sizeof(buf);
    }
};
} // namespace hyperliquid::json