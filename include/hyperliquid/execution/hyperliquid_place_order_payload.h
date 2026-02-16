#pragma once

#include "hyperliquid/json/fixed_json_writer.h"
#include "hyperliquid/market/fixed_point_to_string.h"
#include "hyperliquid/trading/order_request.h"
#include "hyperliquid/trading/types.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace hyperliquid::execution {

// Hyperliquid limit TIF strings (per docs): "Gtc", "Ioc", "Alo" (post-only)
// We’ll map our generic time_in_force + post_only to these.
inline std::string_view to_hl_tif(const hyperliquid::trading::order_request &r) noexcept {
    if (r.post_only)
        return "Alo";
    switch (r.tif) {
    case hyperliquid::trading::time_in_force::gtc:
        return "Gtc";
    case hyperliquid::trading::time_in_force::ioc:
        return "Ioc";
    case hyperliquid::trading::time_in_force::fok:
        return "Fok"; // if unsupported, adapter will change later
    }
    return "Gtc";
}

// Builds WS "post" wrapper with request.type="action" and payload for action.type="order".
// Signature is injected as raw JSON by caller for now.
// Returns false if buffer too small.
inline bool build_place_order_ws_post(char *out, std::size_t cap, std::size_t &used,
                                      std::uint64_t request_id, std::uint64_t nonce,
                                      std::uint32_t asset,
                                      const hyperliquid::trading::order_request &r,
                                      std::string_view signature_json_object) noexcept {
    hyperliquid::json::fixed_json_writer w(out, cap);

    // Convert price/qty to decimal strings (scale 1e6)
    char pxbuf[48];
    char szbuf[48];

    std::size_t pxlen = 0;
    if (r.type == hyperliquid::trading::order_type::limit) {
        pxlen = hyperliquid::market::to_decimal_1e6(r.price, pxbuf, sizeof(pxbuf));
        if (pxlen == 0)
            return false;
    } else {
        return false;
    }

    const std::size_t szlen = hyperliquid::market::to_decimal_1e6(r.qty, szbuf, sizeof(szbuf));
    if (szlen == 0)
        return false;

    const bool is_buy = (r.s == hyperliquid::trading::side::buy);

    // WS wrapper
    w.push_char('{');
    w.push_quoted("method");
    w.push_char(':');
    w.push_quoted("post");
    w.push_char(',');
    w.push_quoted("id");
    w.push_char(':');
    w.push_u64(request_id);
    w.push_char(',');

    w.push_quoted("request");
    w.push_char(':');
    w.push_char('{');
    w.push_quoted("type");
    w.push_char(':');
    w.push_quoted("action");
    w.push_char(',');
    w.push_quoted("payload");
    w.push_char(':');
    w.push_char('{');

    // action payload
    w.push_quoted("action");
    w.push_char(':');
    w.push_char('{');
    w.push_quoted("type");
    w.push_char(':');
    w.push_quoted("order");
    w.push_char(',');
    w.push_quoted("orders");
    w.push_char(':');
    w.push_char('[');
    w.push_char('{');
    w.push_quoted("a");
    w.push_char(':');
    w.push_i64(static_cast<std::int64_t>(asset));
    w.push_char(',');
    w.push_quoted("b");
    w.push_char(':');
    w.push_sv(is_buy ? "true" : "false");
    w.push_char(',');
    w.push_quoted("p");
    w.push_char(':');
    w.push_quoted(std::string_view(pxbuf, pxlen));
    w.push_char(',');
    w.push_quoted("s");
    w.push_char(':');
    w.push_quoted(std::string_view(szbuf, szlen));
    w.push_char(',');
    w.push_quoted("r");
    w.push_char(':');
    w.push_sv(r.reduce_only ? "true" : "false");
    w.push_char(',');

    // order type object
    w.push_quoted("t");
    w.push_char(':');
    w.push_char('{');
    w.push_quoted("limit");
    w.push_char(':');
    w.push_char('{');
    w.push_quoted("tif");
    w.push_char(':');
    w.push_quoted(to_hl_tif(r));
    w.push_char('}');
    w.push_char('}');

    // optional client order id  if present
    if (r.client_id.value[0] != '\0') {
        w.push_char(',');
        w.push_quoted("c");
        w.push_char(':');
        w.push_quoted(r.client_id.value);
    }

    w.push_char('}');
    w.push_char(']');
    w.push_char('}'); // end action

    w.push_char(',');
    w.push_quoted("nonce");
    w.push_char(':');
    w.push_u64(nonce);
    w.push_char(',');
    w.push_quoted("signature");
    w.push_char(':');
    w.push_sv(signature_json_object);

    w.push_char('}'); // end payload
    w.push_char('}'); // end request
    w.push_char('}'); // end outer

    if (!w.ok())
        return false;

    used = w.size();
    return true;
}

} // namespace hyperliquid::execution