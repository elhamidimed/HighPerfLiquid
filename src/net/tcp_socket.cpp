#include "hyperliquid/net/tcp_socket.h"

namespace hyperliquid::net {

tcp_socket::tcp_socket() : fd_(-1) {}
tcp_socket::~tcp_socket() {
    if (fd_ >= 0)
        ::close(fd_);
}

net_status tcp_socket::connect(std::string_view host, uint16_t port) noexcept {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0)
        return net_status::error;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.data(), &addr.sin_addr) <= 0)
        return net_status::error;

    if (::connect(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
        return net_status::error;

    return net_status::ok;
}

void tcp_socket::close() noexcept {
    if (fd_ >= 0)
        ::close(fd_);
    fd_ = -1;
}

net_status tcp_socket::read(char *buffer, std::size_t len, std::size_t &out_bytes) noexcept {
    ssize_t ret = ::recv(fd_, buffer, len, MSG_DONTWAIT);
    if (ret > 0) {
        out_bytes = static_cast<std::size_t>(ret);
        ;
        return net_status::ok;
    }
    if (ret == 0) {
        out_bytes = 0;
        return net_status::closed;
    }
    out_bytes = 0;
    return net_status::error;
}

net_status tcp_socket::write(const char *buffer, std::size_t len, std::size_t &out_bytes) noexcept {
    ssize_t ret = ::send(fd_, buffer, len, MSG_DONTWAIT);
    if (ret >= 0) {
        out_bytes = static_cast<std::size_t>(ret);
        return net_status::ok;
    }
    out_bytes = 0;
    return net_status::error;
}

} // namespace hyperliquid::net
