#include "hyperliquid/net/tls_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include <cstdio>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace hyperliquid::net {

static void openssl_global_init_once() noexcept {
    static bool inited = false;
    if (!inited) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        inited = true;
    }
}

tls_socket::tls_socket() noexcept = default;

tls_socket::~tls_socket() {
    close();
}

bool tls_socket::init_ctx_() noexcept {
    openssl_global_init_once();

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
        return false;

    // system default CA store.
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        SSL_CTX_free(ctx);
        return false;
    }

    ctx_ = ctx;
    return true;
}

void tls_socket::free_ssl_() noexcept {
    if (ssl_) {
        SSL *s = static_cast<SSL *>(ssl_);
        SSL_shutdown(s);
        SSL_free(s);
        ssl_ = nullptr;
    }
    if (ctx_) {
        SSL_CTX_free(static_cast<SSL_CTX *>(ctx_));
        ctx_ = nullptr;
    }
}

net_status tls_socket::connect(std::string_view host, std::uint16_t port) noexcept {
    close();

    if (!init_ctx_())
        return net_status::error;

    // Resolve host (supports DNS).
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    std::snprintf(portstr, sizeof(portstr), "%u", (unsigned)port);

    addrinfo *res = nullptr;
    const int rc = ::getaddrinfo(host.data(), portstr, &hints, &res);
    if (rc != 0 || !res) {
        free_ssl_();
        return net_status::error;
    }

    int fd = -1;
    for (addrinfo *p = res; p != nullptr; p = p->ai_next) {
        fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0)
            continue;
        if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        ::close(fd);
        fd = -1;
    }
    ::freeaddrinfo(res);

    if (fd < 0) {
        free_ssl_();
        return net_status::error;
    }

    fd_ = fd;

    SSL *ssl = SSL_new(static_cast<SSL_CTX *>(ctx_));
    if (!ssl) {
        close();
        return net_status::error;
    }

    char hostbuf[256];
    const std::size_t n = (host.size() < sizeof(hostbuf) - 1) ? host.size() : (sizeof(hostbuf) - 1);
    std::memcpy(hostbuf, host.data(), n);
    hostbuf[n] = '\0';

    (void)SSL_set_tlsext_host_name(ssl, hostbuf);
    SSL_set_fd(ssl, fd_);

    // Verify certificate
    SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);

    // Handshake
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        ssl_ = nullptr;
        close();
        return net_status::error;
    }

    ssl_ = ssl;
    return net_status::ok;
}

void tls_socket::close() noexcept {
    free_ssl_();
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

net_status tls_socket::read(char *buffer, std::size_t len, std::size_t &out_bytes) noexcept {
    out_bytes = 0;
    if (!ssl_)
        return net_status::error;

    SSL *ssl = static_cast<SSL *>(ssl_);
    const int ret = SSL_read(ssl, buffer, static_cast<int>(len));
    if (ret > 0) {
        out_bytes = static_cast<std::size_t>(ret);
        return net_status::ok;
    }

    const int err = SSL_get_error(ssl, ret);
    if (err == SSL_ERROR_ZERO_RETURN)
        return net_status::closed;
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        return net_status::ok;

    return net_status::error;
}

net_status tls_socket::write(const char *buffer, std::size_t len, std::size_t &out_bytes) noexcept {
    out_bytes = 0;
    if (!ssl_)
        return net_status::error;

    SSL *ssl = static_cast<SSL *>(ssl_);
    const int ret = SSL_write(ssl, buffer, static_cast<int>(len));
    if (ret > 0) {
        out_bytes = static_cast<std::size_t>(ret);
        return net_status::ok;
    }

    const int err = SSL_get_error(ssl, ret);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        return net_status::ok;
    if (err == SSL_ERROR_ZERO_RETURN)
        return net_status::closed;

    return net_status::error;
}

} // namespace hyperliquid::net