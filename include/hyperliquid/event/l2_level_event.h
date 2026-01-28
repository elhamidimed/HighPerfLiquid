#pragma once

#include <cstdint>

namespace hyperliquid::event {

enum class side : std::uint8_t { bid = 0, ask = 1 };

struct l2_level_event {
    char symbol[6];
    side level_side; // bid / ask
    int64_t price;
    int64_t size;
    uint32_t order_count; // 'n'
    uint64_t exchange_time_ms;
};

} // namespace hyperliquid::event
