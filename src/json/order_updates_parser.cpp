#include "hyperliquid/json/order_updates_parser.h"

#include "hyperliquid/json/padded_msg.h"
#include "hyperliquid/market/symbol.h"

#include <simdjson.h>

#include <cstring>

#ifndef HPL_ORDER_UPDATES_DEBUG
#define HPL_ORDER_UPDATES_DEBUG 0
#endif

namespace hyperliquid::json {

template <std::size_t InN, std::size_t OutN>
order_updates_parser<InN, OutN>::order_updates_parser(in_buffer_t &in, out_buffer_t &out) noexcept
    : in_(in), out_(out) {}

static inline void set_venue_id_from_oid(hyperliquid::trading::venue_order_id &vid,
                                         std::uint64_t oid) noexcept {
    // decimal into fixed buffer
    std::memset(vid.value, 0, sizeof(vid.value));
    char tmp[32];
    std::size_t n = 0;
    do {
        tmp[n++] = char('0' + (oid % 10));
        oid /= 10;
    } while (oid && n < sizeof(tmp));

    std::size_t out = (n < sizeof(vid.value) - 1) ? n : (sizeof(vid.value) - 1);
    for (std::size_t i = 0; i < out; ++i)
        vid.value[i] = tmp[out - 1 - i];
}

static inline bool status_is_reject(std::string_view s) noexcept {
    // checking substring "reject"
    return s.find("reject") != std::string_view::npos;
}

template <std::size_t InN, std::size_t OutN> void order_updates_parser<InN, OutN>::poll() noexcept {
    simdjson::ondemand::parser parser;

    hyperliquid::event::market_event me{};
#if HPL_ORDER_UPDATES_DEBUG
    std::size_t seen = 0, bad_json = 0, bad_channel = 0, bad_data = 0, emitted = 0;
#endif
    while (in_.pop(me)) {
#if HPL_ORDER_UPDATES_DEBUG
        ++seen;
#endif

        // might adjust size later
        padded_msg<4096> msg{};
        if (!msg.load(me.data.data(), me.size)) {
            continue; // dropping oversized
        }

        auto doc_res = parser.iterate(msg.view());
        if (doc_res.error()) {
#if HPL_ORDER_UPDATES_DEBUG
            ++bad_json;
#endif
            continue;
        }
        simdjson::ondemand::document &doc = doc_res.value_unsafe();

        // channel check
        auto ch = doc["channel"].get_string();
        if (ch.error()) {
#if HPL_ORDER_UPDATES_DEBUG
            ++bad_channel;
#endif
            continue;
        }

        const std::string_view channel = ch.value_unsafe();
        if (channel != "orderUpdates")
            continue;

        // data is WsOrder[]
        auto data = doc["data"].get_array();
        if (data.error()) {
#if HPL_ORDER_UPDATES_DEBUG
            ++bad_data;
#endif
            continue;
        }

        for (auto wsorder : data.value_unsafe()) {
            // status + statusTimestamp
            auto st = wsorder["status"].get_string();
            if (st.error())
                continue;
            const std::string_view status = st.value_unsafe();

            auto ts = wsorder["statusTimestamp"].get_uint64();
            if (ts.error())
                continue;
            const std::uint64_t status_ts = ts.value_unsafe();

            // basic order
            auto ord = wsorder["order"].get_object();
            if (ord.error())
                continue;
            auto o = ord.value_unsafe();

            // cloid may be missing
            hyperliquid::trading::client_order_id cid{};
            std::memset(cid.value, 0, sizeof(cid.value));
            auto cl = o["cloid"].get_string();
            if (!cl.error()) {
                const std::string_view sv = cl.value_unsafe();
                const std::size_t n =
                    (sv.size() < sizeof(cid.value) - 1) ? sv.size() : (sizeof(cid.value) - 1);
                std::memcpy(cid.value, sv.data(), n);
                cid.value[n] = '\0';
            }

            // oid required
            auto oidv = o["oid"].get_uint64();
            if (oidv.error())
                continue;
            const std::uint64_t oid = oidv.value_unsafe();

            hyperliquid::trading::order_event ev{};
            std::memset(&ev, 0, sizeof(ev));

            if (status == "open") {
                ev.type = hyperliquid::trading::order_event_type::ack;
                ev.ack.client_id = cid;
                set_venue_id_from_oid(ev.ack.venue_id, oid);
                ev.ack.exchange_time_ms = status_ts;

                out_.push(ev);
#if HPL_ORDER_UPDATES_DEBUG
                ++emitted;
#endif
            } else if (status_is_reject(status)) {
                ev.type = hyperliquid::trading::order_event_type::reject;
                ev.reject.client_id = cid;
                ev.reject.reason = hyperliquid::trading::reject_reason::venue;
                ev.reject.exchange_time_ms = status_ts;

                // copy status into message
                std::memset(ev.reject.message, 0, sizeof(ev.reject.message));
                const std::size_t n = (status.size() < sizeof(ev.reject.message) - 1)
                                          ? status.size()
                                          : (sizeof(ev.reject.message) - 1);
                std::memcpy(ev.reject.message, status.data(), n);

                out_.push(ev);
#if HPL_ORDER_UPDATES_DEBUG
                ++emitted;
#endif
            } else {
                // ignore other statuses for now, might adjust later
            }
        }
    }
#if HPL_ORDER_UPDATES_DEBUG
    if (emitted == 0) {
        std::printf("[order_updates_parser] seen=%zu bad_json=%zu bad_channel=%zu bad_data=%zu "
                    "emitted=%zu\n",
                    seen, bad_json, bad_channel, bad_data, emitted);
    }
#endif
}

// explicit instantiations
template class order_updates_parser<16, 1024>;

} // namespace hyperliquid::json