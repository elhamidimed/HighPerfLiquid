#include "hyperliquid/json/user_fills_parser.h"

#include "hyperliquid/json/fixed_point.h"
#include "hyperliquid/json/padded_msg.h"

#include <cstring>
#include <simdjson.h>

namespace hyperliquid::json {

template <std::size_t InN, std::size_t OutN>
user_fills_parser<InN, OutN>::user_fills_parser(in_buffer_t &in, out_buffer_t &out) noexcept
    : in_(in), out_(out) {}

static inline void set_venue_id_from_oid(hyperliquid::trading::venue_order_id &vid,
                                         std::uint64_t oid) noexcept {
    std::memset(vid.value, 0, sizeof(vid.value));
    char tmp[32];
    std::size_t n = 0;
    do {
        tmp[n++] = char('0' + (oid % 10));
        oid /= 10;
    } while (oid && n < sizeof(tmp));
    const std::size_t out = (n < sizeof(vid.value) - 1) ? n : (sizeof(vid.value) - 1);
    for (std::size_t i = 0; i < out; ++i)
        vid.value[i] = tmp[out - 1 - i];
}

template <std::size_t InN, std::size_t OutN> void user_fills_parser<InN, OutN>::poll() noexcept {
    simdjson::ondemand::parser parser;

    hyperliquid::event::market_event me{};
    while (in_.pop(me)) {
        padded_msg<4096> msg{};
        if (!msg.load(me.data.data(), me.size))
            continue;

        auto doc_res = parser.iterate(msg.data(), msg.size(), msg.capacity());
        if (doc_res.error())
            continue;
        simdjson::ondemand::document &doc = doc_res.value_unsafe();

        auto ch = doc["channel"].get_string();
        if (ch.error())
            continue;
        const std::string_view channel = ch.value_unsafe();
        if (channel != "userFills")
            continue;

        auto data = doc["data"].get_array();
        if (data.error())
            continue;

        for (auto fill : data.value_unsafe()) {
            // time
            auto t = fill["time"].get_uint64();
            if (t.error())
                continue;
            const std::uint64_t tms = t.value_unsafe();

            // oid
            auto oidv = fill["oid"].get_uint64();
            if (oidv.error())
                continue;
            const std::uint64_t oid = oidv.value_unsafe();

            // side: "B" / "S" typically
            auto sidev = fill["side"].get_string();
            if (sidev.error())
                continue;
            const std::string_view ss = sidev.value_unsafe();
            const hyperliquid::trading::side s =
                (ss == "B") ? hyperliquid::trading::side::buy : hyperliquid::trading::side::sell;

            // price/size
            auto px = fill["px"].get_string();
            auto sz = fill["sz"].get_string();
            if (px.error() || sz.error())
                continue;

            const std::string_view pxs = px.value_unsafe();
            const std::string_view szs = sz.value_unsafe();

            std::int64_t px_i = 0;
            std::int64_t sz_i = 0;
            const hyperliquid::market::price_t fill_px =
                parse_decimal_1e6(pxs.data(), pxs.size(), px_i);
            const hyperliquid::market::qty_t fill_sz =
                parse_decimal_1e6(szs.data(), szs.size(), sz_i);

            // optional cloid
            hyperliquid::trading::client_order_id cid{};
            std::memset(cid.value, 0, sizeof(cid.value));
            auto cl = fill["cloid"].get_string();
            if (!cl.error()) {
                const std::string_view sv = cl.value_unsafe();
                const std::size_t n =
                    (sv.size() < sizeof(cid.value) - 1) ? sv.size() : (sizeof(cid.value) - 1);
                std::memcpy(cid.value, sv.data(), n);
                cid.value[n] = '\0';
            }

            hyperliquid::trading::order_event ev{};
            std::memset(&ev, 0, sizeof(ev));
            ev.type = hyperliquid::trading::order_event_type::fill;
            ev.fill.client_id = cid;
            set_venue_id_from_oid(ev.fill.venue_id, oid);
            ev.fill.s = s;
            ev.fill.fill_price = fill_px;
            ev.fill.fill_qty = fill_sz;
            ev.fill.cum_qty = 0;
            ev.fill.exchange_time_ms = tms;

            out_.push(ev);
        }
    }
}

template class user_fills_parser<16, 1024>;

} // namespace hyperliquid::json