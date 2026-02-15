#pragma once
#include <array>
#include <cstddef>

namespace hyperliquid::event {

struct market_event {
    std::array<char, 8192> data{};
    std::size_t size = 0;
};

} // namespace hyperliquid::event
