#pragma once

#include <cstdint>
#include <string_view>

namespace hyperliquid::config {

struct library_config {

    std::string_view log_level = "info";

    std::uint32_t network_timeout_ms = 500;
};

} // namespace hyperliquid::config
