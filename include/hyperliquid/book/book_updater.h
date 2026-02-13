#pragma once

#include "hyperliquid/book/l2_book.h"
#include "hyperliquid/book/l2_snapshot_builder.h"
#include "hyperliquid/event/l2_level_event.h"
#include "hyperliquid/event/ring_buffer.h"

#include <cstddef>

namespace hyperliquid::book {

template <std::size_t InN, std::size_t Depth> class book_updater {
  public:
    using in_buffer_t = hyperliquid::event::ring_buffer<hyperliquid::event::l2_level_event, InN>;

    book_updater(in_buffer_t &in, l2_book<Depth> &book) noexcept : in_(in), builder_(book) {}

    void poll() noexcept {
        hyperliquid::event::l2_level_event ev{};
        while (in_.pop(ev)) {
            builder_.on_level(ev);
        }
        // For now (snapshot-based), we flush after draining input so book updates become visible.
        builder_.flush();
    }

  private:
    in_buffer_t &in_;
    l2_snapshot_builder<Depth> builder_;
};

} // namespace hyperliquid::book