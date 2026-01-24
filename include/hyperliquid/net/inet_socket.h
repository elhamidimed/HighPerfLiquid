#pragma once

#include <cstdint>
#include <string_view>

namespace hyperliquid::net {

enum class net_status : uint8_t { ok, closed, error };

class inet_socket {
  public:
    virtual ~inet_socket() = default;

    virtual net_status connect(std::string_view host, uint16_t port) noexcept = 0;

    virtual void close() noexcept = 0;

    // Non-blocking read
    virtual net_status read(char *buffer, std::size_t len, std::size_t &out_bytes) noexcept = 0;

    // Non-blocking write
    virtual net_status write(const char *buffer, std::size_t len,
                             std::size_t &out_bytes) noexcept = 0;
};

} // namespace hyperliquid::net
