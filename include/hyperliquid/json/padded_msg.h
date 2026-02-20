#pragma once

#include <array>
#include <cstddef>
#include <cstring>

#include <simdjson.h>

namespace hyperliquid::json {

// Fixed buffer for simdjson: input + required padding.
// (No heap; per-message stack copy.)

template <std::size_t Max> struct padded_msg {

    bool load(const char *src, size_t n) noexcept {
        if (n > Max)
            return false;
        len = n;
        std::memcpy(buf, src, len);
        std::memset(buf + len, 0, simdjson::SIMDJSON_PADDING);
        return true;
    }

    constexpr size_t capacity() const noexcept {
        return sizeof(buf);
    }
    const char *data() const noexcept {
        return buf;
    }
    std::size_t size() const noexcept {
        return len;
    }

    char buf[Max + simdjson::SIMDJSON_PADDING];
    size_t len{0};
};
} // namespace hyperliquid::json