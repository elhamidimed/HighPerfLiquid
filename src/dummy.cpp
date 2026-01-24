#include "hyperliquid/core/timestamp.h"
#include <iostream>

namespace hyperliquid {

void dummy() {
    auto t = core::now_ns();
    std::cout << "now_ns = " << t << '\n';
}

} // namespace hyperliquid
