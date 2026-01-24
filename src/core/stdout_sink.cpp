#include "hyperliquid/core/logger.h"

#include <cstdio>

namespace hyperliquid::core {

class stdout_sink final : public log_sink {
  public:
    void write(log_level, std::string_view msg) noexcept override {
        std::fwrite(msg.data(), 1, msg.size(), stdout);
        std::fwrite("\n", 1, 1, stdout);
    }
};

} // namespace hyperliquid::core
