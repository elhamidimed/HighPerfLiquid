#pragma once

#include "hyperliquid/config/config.h"

#include <string_view>

namespace hyperliquid::config {
library_config load_from_file(std::string_view filename) noexcept;
}