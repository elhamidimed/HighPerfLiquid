#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hyperliquid::latency {

// Log2 histogram for nanoseconds.
// Buckets represent ranges: [1,2), [2,4), [4,8), ... in ns.
// Bucket 0 is for 0 ns.
// Last bucket is overflow.
template <std::size_t Buckets = 64> class log2_histogram {
  public:
    void reset() noexcept {
        counts_.fill(0);
        total_ = 0;
        sum_ = 0;
        min_ = std::numeric_limits<std::int64_t>::max();
        max_ = 0;
    }

    void record_ns(std::int64_t ns) noexcept {
        if (ns < 0)
            ns = 0;
        const std::size_t b = bucket_index(static_cast<std::uint64_t>(ns));
        counts_[b] += 1;
        total_ += 1;
        sum_ += static_cast<std::uint64_t>(ns);
        if (ns < min_)
            min_ = ns;
        if (ns > max_)
            max_ = ns;
    }

    std::uint64_t count() const noexcept {
        return total_;
    }
    std::int64_t min_ns() const noexcept {
        return (total_ == 0) ? 0 : min_;
    }
    std::int64_t max_ns() const noexcept {
        return (total_ == 0) ? 0 : max_;
    }

    double mean_ns() const noexcept {
        if (total_ == 0)
            return 0.0;
        return static_cast<double>(sum_) / static_cast<double>(total_);
    }

    // Percentile estimate: returns bucket lower-bound in ns.
    // p in [0,1]. Example: 0.99 => p99.
    std::int64_t percentile_ns(double p) const noexcept {
        if (total_ == 0)
            return 0;
        if (p <= 0.0)
            return min_ns();
        if (p >= 1.0)
            return max_ns();

        const std::uint64_t target =
            static_cast<std::uint64_t>(p * static_cast<double>(total_ - 1));
        std::uint64_t seen = 0;
        for (std::size_t i = 0; i < Buckets; ++i) {
            const std::uint64_t c = counts_[i];
            if (c == 0)
                continue;
            if (seen + c > target) {
                return bucket_lower_bound_ns(i);
            }
            seen += c;
        }
        return max_ns();
    }

  private:
    static constexpr std::size_t last = Buckets - 1;

    static std::size_t bucket_index(std::uint64_t ns) noexcept {
        if (ns == 0)
            return 0;
        // compute floor(log2(ns)) + 1, clamp
        std::size_t idx = 0;
        std::uint64_t v = ns;
        while (v >>= 1) {
            ++idx;
        }         // idx = floor(log2(ns))
        idx += 1; // bucket 1 => [1,2), bucket 2 => [2,4), ...
        if (idx >= Buckets)
            return last;
        return idx;
    }

    static std::int64_t bucket_lower_bound_ns(std::size_t idx) noexcept {
        if (idx == 0)
            return 0;
        if (idx >= Buckets)
            return (1LL << (Buckets - 2)); // conservative
        return (1LL << (idx - 1));
    }

    std::array<std::uint64_t, Buckets> counts_{};
    std::uint64_t total_{0};
    std::uint64_t sum_{0};
    std::int64_t min_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t max_{0};
};

} // namespace hyperliquid::latency