#pragma once

#include "hyperliquid/core/log_level.h"

#include <string_view>

namespace hyperliquid::core {

class log_sink {
  public:
    virtual ~log_sink() = default;

    virtual void write(log_level level, std::string_view message) noexcept = 0;
};

} // namespace hyperliquid::core
