#include "hyperliquid/ws/websocket_client.h"
#include <cstring>
#include <iostream>

namespace hyperliquid::ws {

template <typename Event, std::size_t N> class dummy_ws_client : public websocket_client<Event, N> {
  public:
    using websocket_client<Event, N>::buffer_;

    dummy_ws_client(hyperliquid::event::ring_buffer<Event, N> &buf)
        : websocket_client<Event, N>(buf) {}

    hyperliquid::net::net_status connect(std::string_view) noexcept override {
        return hyperliquid::net::net_status::ok;
    }

    void poll() noexcept override {
        // Simulate receiving a message
        Event evt{};
        evt.data[0] = 'H';
        evt.data[1] = 'i';
        evt.size = 2;
        this->buffer_.push(evt);
    }
};

} // namespace hyperliquid::ws
