#pragma once

#include <atomic>
#include <cstddef>
#include <stdexcept>

namespace hyperliquid::event {

template <typename T, std::size_t N> class ring_buffer {
  public:
    static_assert(N > 0, "Ring buffer size must be positive");

    ring_buffer() : head_(0), tail_(0) {}

    bool push(const T &item) noexcept {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next_head = increment(head);
        if (next_head == tail_.load(std::memory_order_acquire))
            return false; // full
        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T &item) noexcept {
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire))
            return false; // empty
        item = buffer_[tail];
        tail_.store(increment(tail), std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load() == tail_.load();
    }
    bool full() const noexcept {
        return increment(head_.load()) == tail_.load();
    }
    size_t capacity() const noexcept {
        return N - 1;
    } // one slot wasted

  private:
    size_t increment(size_t idx) const noexcept {
        return (idx + 1) % N;
    }

    T buffer_[N];
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

} // namespace hyperliquid::event
