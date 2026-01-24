#include "hyperliquid/config/config.h"

namespace hyperliquid::config {

library_config load_from_file(std::string_view /*filename*/) noexcept {
    library_config cfg{};
    // TODO: implement JSON / file parsing
    return cfg;
}

} // namespace hyperliquid::config
