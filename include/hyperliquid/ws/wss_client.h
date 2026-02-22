#pragma once

#include "hyperliquid/net/inet_socket.h"
#include "hyperliquid/net/tls_socket.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace hyperliquid::ws {

enum class ws_status : std::uint8_t { ok = 0, closed = 1, error = 2, would_block = 3 };

struct text_view {
    const char *data;
    std::size_t size;
};

class wss_client {
  public:
    wss_client() noexcept = default;

    // Connect + WS handshake
    bool connect(std::string_view host, std::string_view path, std::uint16_t port = 443) noexcept;

    void close() noexcept;

    bool send_text(std::string_view text) noexcept;

    // Poll socket and process one frame. If a text frame is received, returns ws_status::ok and
    ws_status poll(text_view &out) noexcept;

  private:
    net::tls_socket sock_;

    // handshake state
    bool open_{false};

    static constexpr std::size_t RX_CAP = 1 << 16;
    char rx_[RX_CAP];
    std::size_t rx_len_{0};

    static constexpr std::size_t MSG_CAP = 1 << 16;
    char msg_[MSG_CAP];
    std::size_t msg_len_{0};

    bool handshake_(std::string_view host, std::string_view path) noexcept;

    // helpers
    ws_status parse_one_frame_(text_view &out) noexcept;
    bool write_all_(const char *p, std::size_t n) noexcept;

    // utilities
    static void random_key_b64_(char out_key[25]) noexcept; // 24 chars + '\0'
    static bool compute_accept_(std::string_view key_b64,
                                char out_accept_b64[29]) noexcept; // 28 + '\0'
    static bool header_value_(std::string_view hdrs, std::string_view name,
                              std::string_view &value) noexcept;
};

} // namespace hyperliquid::ws