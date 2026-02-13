#pragma once

#include "hyperliquid/latency/clock.h"

namespace hyperliquid::latency {

// RAII scope helper
template <typename Sink> class scope_timer {
  public:
    explicit scope_timer(Sink &sink) noexcept : sink_(sink), start_(now()) {}

    ~scope_timer() noexcept {
        const auto end = now();
        sink_.record_ns(ns_since(start_, end));
    }

    scope_timer(const scope_timer &) = delete;
    scope_timer &operator=(const scope_timer &) = delete;

  private:
    Sink &sink_;
    time_point start_;
};

} // namespace hyperliquid::latency