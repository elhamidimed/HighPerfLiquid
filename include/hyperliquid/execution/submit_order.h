#pragma once

#include "hyperliquid/execution/hyperliquid_place_order_payload.h"
#include "hyperliquid/risk/risk_engine.h"
#include "hyperliquid/trading/order_request.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace hyperliquid::execution {

struct submit_result {
    hyperliquid::risk::decision decision;
    std::size_t json_bytes;
};

// If allowed, builds JSON into out buffer. if denied, json_bytes=0.
inline submit_result submit_order(const hyperliquid::risk::risk_engine &risk, char *out,
                                  std::size_t cap, std::uint64_t request_id, std::uint64_t nonce,
                                  std::uint32_t asset, const hyperliquid::trading::order_request &r,
                                  std::string_view signature_json_object) noexcept {
    submit_result res{};
    res.decision = risk.check(r);
    res.json_bytes = 0;

    if (!res.decision.allow) {
        return res;
    }

    std::size_t used = 0;
    const bool ok = hyperliquid::execution::build_place_order_ws_post(
        out, cap, used, request_id, nonce, asset, r, signature_json_object);

    if (!ok) {
        // treat builder failure as deny-like outcome, but keep reason as ok.
        used = 0;
    }

    res.json_bytes = used;
    return res;
}

} // namespace hyperliquid::execution