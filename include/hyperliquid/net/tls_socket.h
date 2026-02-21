#pragma once

#include "hyperliquid/net/inet_socket.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace hyperliquid::net {

class tls_socket final : public inet_socket {
  public:
    tls_socket() noexcept;
    ~tls_socket() override;

    net_status connect(std::string_view host, std::uint16_t port) noexcept override;
    void close() noexcept override;

    net_status read(char *buffer, std::size_t len, std::size_t &out_bytes) noexcept override;
    net_status write(const char *buffer, std::size_t len, std::size_t &out_bytes) noexcept override;

  private:
    int fd_{-1};
    void *ctx_{nullptr}; // SSL_CTX*
    void *ssl_{nullptr}; // SSL*

    bool init_ctx_() noexcept;
    void free_ssl_() noexcept;
};

} // namespace hyperliquid::net