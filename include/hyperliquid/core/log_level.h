#pragma once

#include <cstdint>

namespace hyperliquid::core {

enum class log_level : std::uint8_t { trace = 0, debug, info, warn, error, fatal };

} // namespace hyperliquid::core
