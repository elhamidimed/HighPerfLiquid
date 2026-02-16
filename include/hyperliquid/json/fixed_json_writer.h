#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace hyperliquid::json {

// Minimal fixed-buffer JSON writer (we emit our own JSON)
class fixed_json_writer {
  public:
    fixed_json_writer(char *out, std::size_t cap) noexcept : out_(out), cap_(cap) {}

    bool ok() const noexcept {
        return ok_;
    }
    std::size_t size() const noexcept {
        return pos_;
    }
    char *data() noexcept {
        return out_;
    }
    const char *data() const noexcept {
        return out_;
    }

    bool push_char(char c) noexcept {
        if (pos_ >= cap_)
            return fail();
        out_[pos_++] = c;
        return true;
    }

    bool push_sv(std::string_view s) noexcept {
        if (pos_ + s.size() > cap_)
            return fail();
        std::memcpy(out_ + pos_, s.data(), s.size());
        pos_ += s.size();
        return true;
    }

    bool push_u64(std::uint64_t v) noexcept {
        char buf[32];
        std::size_t n = u64_to_buf(v, buf);
        return push_sv(std::string_view(buf, n));
    }

    bool push_i64(std::int64_t v) noexcept {
        char buf[32];
        std::size_t n = i64_to_buf(v, buf);
        return push_sv(std::string_view(buf, n));
    }

    // Writes "...." with minimal escaping for quotes/backslash/control
    bool push_quoted(std::string_view s) noexcept {
        if (!push_char('"'))
            return false;
        for (char ch : s) {
            switch (ch) {
            case '\"':
                if (!push_sv("\\\""))
                    return false;
                break;
            case '\\':
                if (!push_sv("\\\\"))
                    return false;
                break;
            case '\b':
                if (!push_sv("\\b"))
                    return false;
                break;
            case '\f':
                if (!push_sv("\\f"))
                    return false;
                break;
            case '\n':
                if (!push_sv("\\n"))
                    return false;
                break;
            case '\r':
                if (!push_sv("\\r"))
                    return false;
                break;
            case '\t':
                if (!push_sv("\\t"))
                    return false;
                break;
            default:
                // control chars not expected; if they happen, fail fast
                if (static_cast<unsigned char>(ch) < 0x20)
                    return fail();
                if (!push_char(ch))
                    return false;
                break;
            }
        }
        return push_char('"');
    }

  private:
    bool fail() noexcept {
        ok_ = false;
        return false;
    }

    static std::size_t u64_to_buf(std::uint64_t v, char *out) noexcept {
        char tmp[32];
        std::size_t n = 0;
        do {
            tmp[n++] = char('0' + (v % 10));
            v /= 10;
        } while (v);
        // reverse
        for (std::size_t i = 0; i < n; ++i)
            out[i] = tmp[n - 1 - i];
        return n;
    }

    static std::size_t i64_to_buf(std::int64_t v, char *out) noexcept {
        std::uint64_t u = static_cast<std::uint64_t>(v);
        std::size_t pos = 0;
        if (v < 0) {
            out[pos++] = '-';
            // careful: negate via unsigned
            u = static_cast<std::uint64_t>(-(v + 1)) + 1;
        }
        char tmp[32];
        std::size_t n = 0;
        do {
            tmp[n++] = char('0' + (u % 10));
            u /= 10;
        } while (u);
        for (std::size_t i = 0; i < n; ++i)
            out[pos + i] = tmp[n - 1 - i];
        return pos + n;
    }

    char *out_{nullptr};
    std::size_t cap_{0};
    std::size_t pos_{0};
    bool ok_{true};
};

} // namespace hyperliquid::json