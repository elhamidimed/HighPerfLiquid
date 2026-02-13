#include "hyperliquid/json/l2_book_parser.h"
#include "hyperliquid/json/fixed_point.h"

#include <simdjson.h>

#include <cstdint>
#include <cstring>

namespace hyperliquid::json {

namespace {

// Fixed buffer for simdjson: input + required padding.
// (No heap; per-message stack copy.)

template <std::size_t Max> struct padded_msg {
    char buf[Max + simdjson::SIMDJSON_PADDING];
    size_t len{0};

    void load(const char *src, size_t n) noexcept {
        len = (n > Max) ? Max : n;
        std::memcpy(buf, src, len);
        std::memset(buf + len, 0, simdjson::SIMDJSON_PADDING);
    }

    simdjson::padded_string_view view() const noexcept {
        return simdjson::padded_string_view(buf, len);
    }

    constexpr size_t capacity() const noexcept {
        return sizeof(buf);
    }
};

inline bool copy_symbol_5(char (&dst)[6], std::string_view sym) noexcept {
    if (sym.size() > 5)
        return false; // strict: reject unexpected symbols
    std::memset(dst, 0, sizeof(dst));
    std::memcpy(dst, sym.data(), sym.size());
    dst[sym.size()] = '\0';
    return true;
}

} // namespace

template <std::size_t InN, std::size_t OutN> void l2_book_parser<InN, OutN>::poll() noexcept {

    static thread_local simdjson::ondemand::parser parser{};

    hyperliquid::event::market_event me{};

    while (in_.pop(me)) {
        padded_msg<256> msg{};
        msg.load(me.data.data(), me.size);

        auto doc_res = parser.iterate(msg.buf, msg.len, msg.capacity());
        if (doc_res.error()) {
            continue; // drop invalid JSON
        }
        simdjson::ondemand::document &doc = doc_res.value_unsafe();

        // channel must be "l2Book"
        auto ch_res = doc["channel"].get_string();
        if (ch_res.error())
            continue;
        std::string_view channel = ch_res.value_unsafe();
        if (channel != "l2Book")
            continue;

        auto data_res = doc["data"].get_object();
        if (data_res.error())
            continue;
        auto data = data_res.value_unsafe();

        auto coin_res = data["coin"].get_string();
        if (coin_res.error())
            continue;
        std::string_view coin = coin_res.value_unsafe();

        char symbol[6];
        if (!copy_symbol_5(symbol, coin)) {
            continue; // strict: drop if symbol longer than expected
        }

        auto time_res = data["time"].get_uint64();
        if (time_res.error())
            continue;
        std::uint64_t t_ms = time_res.value_unsafe();

        auto levels_res = data["levels"].get_array();
        if (levels_res.error())
            continue;
        auto levels = levels_res.value_unsafe();

        std::size_t side_idx = 0;
        for (auto side_any : levels) {
            if (side_idx >= 2) {
                // More than 2 sides => invalid schema
                side_idx = 999; // marker invalid
                break;
            }
            auto side_arr_res = side_any.get_array();
            if (side_arr_res.error()) {
                side_idx = 999;
                break;
            }
            auto side_arr = side_arr_res.value_unsafe();

            const hyperliquid::event::side s =
                (side_idx == 0) ? hyperliquid::event::side::bid : hyperliquid::event::side::ask;

            for (auto lvl_any : side_arr) {
                auto lvl_obj_res = lvl_any.get_object();
                if (lvl_obj_res.error())
                    continue;
                auto lvl = lvl_obj_res.value_unsafe();

                auto px_res = lvl["px"].get_string();
                auto sz_res = lvl["sz"].get_string();
                auto n_res = lvl["n"].get_uint64();
                if (px_res.error() || sz_res.error() || n_res.error())
                    continue;

                std::string_view px = px_res.value_unsafe();
                std::string_view sz = sz_res.value_unsafe();
                std::uint64_t n = n_res.value_unsafe();

                std::int64_t px_i = 0;
                std::int64_t sz_i = 0;
                if (!parse_decimal_1e6(px.data(), px.size(), px_i))
                    continue;
                if (!parse_decimal_1e6(sz.data(), sz.size(), sz_i))
                    continue;

                hyperliquid::event::l2_level_event out{};
                std::memcpy(out.symbol.value, symbol, sizeof(out.symbol.value));
                out.level_side = s;
                out.price = px_i;
                out.size = sz_i;
                out.order_count = static_cast<std::uint32_t>(n);
                out.exchange_time_ms = t_ms;

                // If output buffer is full, stop emitting for this message.
                bool out_full = false;

                // inside level loop:
                if (!out_.push(out)) {
                    out_full = true;
                    break;
                }

                // after inner loop:
                if (out_full)
                    break;

                // after finishing sides:
                if (out_full)
                    continue; // move on to next input msg
            }

            ++side_idx;
        }
    }
}

// For the sanity executable, I'll explicitly instantiate one concrete size pair.
template class l2_book_parser<1024, 65536>;
template class l2_book_parser<16, 4096>;

} // namespace hyperliquid::json
