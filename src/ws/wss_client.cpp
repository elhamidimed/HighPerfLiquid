#include "hyperliquid/ws/wss_client.h"

#include <openssl/sha.h>

#include <cstdio>
#include <cstring>

namespace hyperliquid::ws {

static inline bool starts_with_ci(std::string_view s, std::string_view pfx) noexcept {
    if (s.size() < pfx.size())
        return false;
    for (std::size_t i = 0; i < pfx.size(); ++i) {
        char a = s[i], b = pfx[i];
        if (a >= 'A' && a <= 'Z')
            a = char(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z')
            b = char(b - 'A' + 'a');
        if (a != b)
            return false;
    }
    return true;
}

//  base64 encoder for fixed-size input
static inline std::size_t b64_encode(const std::uint8_t *in, std::size_t n, char *out,
                                     std::size_t cap) noexcept {
    static constexpr char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::size_t o = 0;
    std::size_t i = 0;
    while (i + 2 < n) {
        if (o + 4 > cap)
            return 0;
        const std::uint32_t v =
            (std::uint32_t(in[i]) << 16) | (std::uint32_t(in[i + 1]) << 8) | in[i + 2];
        out[o++] = tbl[(v >> 18) & 63];
        out[o++] = tbl[(v >> 12) & 63];
        out[o++] = tbl[(v >> 6) & 63];
        out[o++] = tbl[v & 63];
        i += 3;
    }
    if (i < n) {
        if (o + 4 > cap)
            return 0;
        std::uint32_t v = std::uint32_t(in[i]) << 16;
        if (i + 1 < n)
            v |= std::uint32_t(in[i + 1]) << 8;
        out[o++] = tbl[(v >> 18) & 63];
        out[o++] = tbl[(v >> 12) & 63];
        if (i + 1 < n) {
            out[o++] = tbl[(v >> 6) & 63];
            out[o++] = '=';
        } else {
            out[o++] = '=';
            out[o++] = '=';
        }
    }
    return o;
}

void wss_client::random_key_b64_(char out_key[25]) noexcept {
    // 16 random bytes -> 24 base64 chars
    std::uint8_t rnd[16];
    std::uint64_t x = reinterpret_cast<std::uintptr_t>(&out_key) ^ 0x9e3779b97f4a7c15ULL;
    for (int i = 0; i < 16; ++i) {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        rnd[i] = std::uint8_t(x & 0xFF);
    }
    const std::size_t n = b64_encode(rnd, sizeof(rnd), out_key, 24);
    out_key[n] = '\0';
}

// accept = base64( SHA1( key + GUID ) )
bool wss_client::compute_accept_(std::string_view key_b64, char out_accept_b64[29]) noexcept {
    static constexpr char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    char buf[128];
    const std::size_t need = key_b64.size() + (sizeof(guid) - 1);
    if (need >= sizeof(buf))
        return false;

    std::memcpy(buf, key_b64.data(), key_b64.size());
    std::memcpy(buf + key_b64.size(), guid, sizeof(guid) - 1);

    std::uint8_t digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const std::uint8_t *>(buf), need, digest);

    const std::size_t n = b64_encode(digest, sizeof(digest), out_accept_b64, 28);
    if (n != 28)
        return false;
    out_accept_b64[28] = '\0';
    return true;
}

bool wss_client::header_value_(std::string_view hdrs, std::string_view name,
                               std::string_view &value) noexcept {
    std::size_t pos = 0;
    while (pos < hdrs.size()) {
        const std::size_t eol = hdrs.find("\r\n", pos);
        const std::size_t line_end = (eol == std::string_view::npos) ? hdrs.size() : eol;
        std::string_view line = hdrs.substr(pos, line_end - pos);
        if (starts_with_ci(line, name) && line.size() > name.size() && line[name.size()] == ':') {
            std::size_t vpos = name.size() + 1;
            while (vpos < line.size() && (line[vpos] == ' ' || line[vpos] == '\t'))
                ++vpos;
            value = line.substr(vpos);
            return true;
        }
        if (eol == std::string_view::npos)
            break;
        pos = eol + 2;
    }
    return false;
}

bool wss_client::write_all_(const char *p, std::size_t n) noexcept {
    std::size_t off = 0;
    while (off < n) {
        std::size_t wrote = 0;
        auto st = sock_.write(p + off, n - off, wrote);
        if (st != net::net_status::ok)
            return false;
        if (wrote == 0)
            return false;
        off += wrote;
    }
    return true;
}

bool wss_client::handshake_(std::string_view host, std::string_view path) noexcept {
    char key[25];
    random_key_b64_(key);

    char req[1024];
    int n = std::snprintf(req, sizeof(req),
                          "GET %.*s HTTP/1.1\r\n"
                          "Host: %.*s\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: %s\r\n"
                          "Sec-WebSocket-Version: 13\r\n"
                          "\r\n",
                          (int)path.size(), path.data(), (int)host.size(), host.data(), key);
    if (n <= 0 || (std::size_t)n >= sizeof(req))
        return false;

    if (!write_all_(req, (std::size_t)n))
        return false;

    // read HTTP res until \r\n\r\n
    rx_len_ = 0;
    while (true) {
        if (rx_len_ + 1 >= RX_CAP)
            return false;
        std::size_t got = 0;
        auto st = sock_.read(rx_ + rx_len_, RX_CAP - rx_len_ - 1, got);
        if (st == net::net_status::closed || st == net::net_status::error)
            return false;
        if (got == 0)
            continue;
        rx_len_ += got;
        rx_[rx_len_] = '\0';

        const char *p = std::strstr(rx_, "\r\n\r\n");
        if (p) {
            const std::size_t hdr_len = (p - rx_) + 4;
            std::string_view hdrs(rx_, hdr_len);

            // Must be 101
            if (hdrs.find("101") == std::string_view::npos)
                return false;

            std::string_view accept;
            if (!header_value_(hdrs, "Sec-WebSocket-Accept", accept))
                return false;

            char expected[29];
            if (!compute_accept_(key, expected))
                return false;
            if (accept != expected)
                return false;

            const std::size_t rem = rx_len_ - hdr_len;
            if (rem)
                std::memmove(rx_, rx_ + hdr_len, rem);
            rx_len_ = rem;
            return true;
        }
    }
}

bool wss_client::connect(std::string_view host, std::string_view path,
                         std::uint16_t port) noexcept {
    close();
    if (sock_.connect(host, port) != net::net_status::ok)
        return false;
    if (!handshake_(host, path)) {
        close();
        return false;
    }
    open_ = true;
    return true;
}

void wss_client::close() noexcept {
    open_ = false;
    sock_.close();
    rx_len_ = 0;
    msg_len_ = 0;
}

bool wss_client::send_text(std::string_view text) noexcept {
    if (!open_)
        return false;

    // client frames MUST be masked
    std::uint8_t hdr[14];
    std::size_t hlen = 0;

    hdr[0] = 0x81; // FIN=1, text
    if (text.size() <= 125) {
        hdr[1] = 0x80 | std::uint8_t(text.size());
        hlen = 2;
    } else if (text.size() <= 0xFFFF) {
        hdr[1] = 0x80 | 126;
        hdr[2] = (text.size() >> 8) & 0xFF;
        hdr[3] = (text.size()) & 0xFF;
        hlen = 4;
    } else {
        hdr[1] = 0x80 | 127;
        std::uint64_t L = (std::uint64_t)text.size();
        for (int i = 0; i < 8; ++i)
            hdr[2 + i] = (L >> (56 - 8 * i)) & 0xFF;
        hlen = 10;
    }

    // mask key
    std::uint8_t mask[4];
    std::uint64_t x = reinterpret_cast<std::uintptr_t>(this) ^ 0xa5a5a5a5ULL;
    for (int i = 0; i < 4; ++i) {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        mask[i] = std::uint8_t(x & 0xFF);
    }
    std::memcpy(hdr + hlen, mask, 4);
    hlen += 4;

    if (!write_all_((const char *)hdr, hlen))
        return false;

    // mask on the fly
    char tmp[2048];
    std::size_t off = 0;
    while (off < text.size()) {
        const std::size_t chunk =
            (text.size() - off > sizeof(tmp)) ? sizeof(tmp) : (text.size() - off);
        for (std::size_t i = 0; i < chunk; ++i) {
            tmp[i] = char(text[off + i] ^ mask[(off + i) & 3]);
        }
        if (!write_all_(tmp, chunk))
            return false;
        off += chunk;
    }

    return true;
}

ws_status wss_client::poll(text_view &out) noexcept {
    out = {nullptr, 0};
    if (!open_)
        return ws_status::closed;

    auto st = parse_one_frame_(out);
    if (st == ws_status::ok || st == ws_status::closed || st == ws_status::error)
        return st;

    // Need more bytes
    if (rx_len_ >= RX_CAP)
        return ws_status::error;
    std::size_t got = 0;
    auto r = sock_.read(rx_ + rx_len_, RX_CAP - rx_len_, got);
    if (r == net::net_status::closed)
        return ws_status::closed;
    if (r == net::net_status::error)
        return ws_status::error;
    if (got == 0)
        return ws_status::would_block;
    rx_len_ += got;

    return parse_one_frame_(out);
}

ws_status wss_client::parse_one_frame_(text_view &out) noexcept {
    // Need at least 2 bytes
    if (rx_len_ < 2)
        return ws_status::would_block;

    const std::uint8_t b0 = (std::uint8_t)rx_[0];
    const std::uint8_t b1 = (std::uint8_t)rx_[1];

    const bool fin = (b0 & 0x80) != 0;
    const std::uint8_t opcode = b0 & 0x0F;

    const bool masked = (b1 & 0x80) != 0;
    std::uint64_t len = (b1 & 0x7F);

    std::size_t pos = 2;
    if (len == 126) {
        if (rx_len_ < pos + 2)
            return ws_status::would_block;
        len = ((std::uint64_t)(std::uint8_t)rx_[pos] << 8) | (std::uint8_t)rx_[pos + 1];
        pos += 2;
    } else if (len == 127) {
        if (rx_len_ < pos + 8)
            return ws_status::would_block;
        len = 0;
        for (int i = 0; i < 8; ++i)
            len = (len << 8) | (std::uint8_t)rx_[pos + i];
        pos += 8;
    }

    std::uint8_t mask[4]{0, 0, 0, 0};
    if (masked) {
        if (rx_len_ < pos + 4)
            return ws_status::would_block;
        mask[0] = (std::uint8_t)rx_[pos + 0];
        mask[1] = (std::uint8_t)rx_[pos + 1];
        mask[2] = (std::uint8_t)rx_[pos + 2];
        mask[3] = (std::uint8_t)rx_[pos + 3];
        pos += 4;
    }

    if (rx_len_ < pos + (std::size_t)len)
        return ws_status::would_block;

    const char *payload = rx_ + pos;

    // If masked arrives, we unmask
    msg_len_ = 0;

    if (len > MSG_CAP)
        return ws_status::error;

    if (masked) {
        for (std::size_t i = 0; i < (std::size_t)len; ++i)
            msg_[i] = char(payload[i] ^ mask[i & 3]);
    } else {
        std::memcpy(msg_, payload, (std::size_t)len);
    }
    msg_len_ = (std::size_t)len;

    // consume frame from rx buffer
    const std::size_t frame_bytes = pos + (std::size_t)len;
    const std::size_t rem = rx_len_ - frame_bytes;
    if (rem)
        std::memmove(rx_, rx_ + frame_bytes, rem);
    rx_len_ = rem;

    if (opcode == 0x8) { // close
        open_ = false;
        return ws_status::closed;
    }
    if (opcode == 0x9) { // ping -> pong
        // ignore payload and send empty pong
        //  (good enough; can improve later)
        (void)send_text(""); // NOT correct pong, fix
        return ws_status::would_block;
    }
    if (opcode == 0xA) { // pong
        return ws_status::would_block;
    }

    if (!fin) {
        return ws_status::error;
    }

    if (opcode == 0x1) { // text
        out = {msg_, msg_len_};
        return ws_status::ok;
    }

    // ignore other opcodes
    return ws_status::would_block;
}

} // namespace hyperliquid::ws