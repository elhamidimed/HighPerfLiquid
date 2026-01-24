#pragma once
#include "inet_socket.h"

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

namespace hyperliquid::net {

class tcp_socket : public inet_socket {
  public:
    tcp_socket();
    ~tcp_socket() override;

    net_status connect(std::string_view host, uint16_t port) noexcept override;
    void close() noexcept override;
    net_status read(char *buffer, std::size_t len, std::size_t &out_bytes) noexcept override;
    net_status write(const char *buffer, std::size_t len, std::size_t &out_bytes) noexcept override;

  private:
    int fd_;
};

} // namespace hyperliquid::net
