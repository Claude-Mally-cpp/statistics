#pragma once
#include "HighPrecisionFloat.hpp"
#include "numeric.hpp"

#include <array>
#include <algorithm>
#include <type_traits>
#include <ranges>

namespace mally::statlib {

struct QuartileSummary {
    HighPrecisionFloat q1{};
    HighPrecisionFloat median{};
    HighPrecisionFloat q3{};
};

namespace detail {

// Median of an *already-sorted* inclusive slice [lo..hi]
template <std::size_t N>
constexpr HighPrecisionFloat median_sorted_slice(const std::array<HighPrecisionFloat, N>& a,
                                                 std::size_t lo, std::size_t hi)
{
    const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
    if (len == 0) {
        return 0.0L;
    } else if (len & 1U) { // odd
        return a[lo + len / 2];
    } else {               // even
        const std::size_t mid_hi = lo + len / 2;
        const std::size_t mid_lo = mid_hi - 1;
        return (a[mid_lo] + a[mid_hi]) / 2.0L;
    }
}

// Median of an *already-sorted* whole array
template <std::size_t N>
constexpr HighPrecisionFloat median_sorted(const std::array<HighPrecisionFloat, N>& a)
{
    if constexpr (N == 0) {
        return 0.0L;
    } else if constexpr (N % 2 == 1) {
        return a[N / 2];
    } else {
        return (a[N / 2 - 1] + a[N / 2]) / 2.0L;
    }
}

} // namespace detail

// Tukey hinges on an *already-sorted* array<HPF, N>
template <std::size_t N>
constexpr QuartileSummary quartiles_sorted(const std::array<HighPrecisionFloat, N>& sorted)
{
    if constexpr (N == 0) {
        return {};
    } else {
        const HighPrecisionFloat med = detail::median_sorted(sorted);

        std::size_t loL = 0, loH, hiL, hiH = N - 1;
        if constexpr (N % 2 == 1) {
            const std::size_t mid = N / 2;
            loH = mid;   // [0 .. mid]
            hiL = mid;   // [mid .. N-1]
        } else {
            loH = N / 2 - 1; // [0 .. N/2 - 1]
            hiL = N / 2;     // [N/2 .. N-1]
        }

        const HighPrecisionFloat q1 = detail::median_sorted_slice(sorted, loL, loH);
        const HighPrecisionFloat q3 = detail::median_sorted_slice(sorted, hiL, hiH);
        return {q1, med, q3};
    }
}

// Public API: array-only path (no vectors).
template <class T, std::size_t N>
    requires (std::is_arithmetic_v<std::remove_cvref_t<T>> ||
              std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr QuartileSummary quartiles(const std::array<T, N>& data)
{
    if constexpr (N == 0) {
        return {};
    } else {
        std::array<HighPrecisionFloat, N> hp{};
        for (std::size_t i = 0; i < N; ++i) hp[i] = toHPF(data[i]);
        std::ranges::sort(hp);
        return quartiles_sorted(hp);
    }
}

} // namespace mally::statlib
