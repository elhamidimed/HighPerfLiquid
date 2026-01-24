#pragma once

#include <cstddef>
#include <stdexcept>

namespace hyperliquid::memory {

template <typename T, std::size_t N> class fixed_buffer {
  public:
    fixed_buffer() : size_(0) {}

    void push_back(const T &value) {
        if (size_ >= N)
            throw std::runtime_error("Buffer overflow");
        data_[size_++] = value;
    }

    T &operator[](std::size_t idx) {
        return data_[idx];
    }
    const T &operator[](std::size_t idx) const {
        return data_[idx];
    }

    std::size_t size() const {
        return size_;
    }
    std::size_t capacity() const {
        return N;
    }

  private:
    T data_[N];
    std::size_t size_;
};

} // namespace hyperliquid::memory
