#pragma once

#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/net/inet_socket.h"
#include <string_view>

namespace hyperliquid::ws {

template <typename Event, std::size_t N> class websocket_client {
  public:
    websocket_client(hyperliquid::event::ring_buffer<Event, N> &out_buffer) : buffer_(out_buffer) {}

    //(blocking for now)
    virtual hyperliquid::net::net_status connect(std::string_view uri) noexcept = 0;

    // Poll for messages and push them into ring buffer
    virtual void poll() noexcept = 0;

  protected:
    hyperliquid::event::ring_buffer<Event, N> &buffer_;
};

} // namespace hyperliquid::ws
