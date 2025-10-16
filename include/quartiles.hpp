#pragma once
#include "HighPrecisionFloat.hpp"
#include "numeric.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <type_traits>
#include <vector>

namespace mally::statlib {

/// @brief Q1/median/Q3 summary.
struct QuartileSummary {
    HighPrecisionFloat q1{};
    HighPrecisionFloat median{};
    HighPrecisionFloat q3{};
};

namespace detail {

/// @brief Median of an already-sorted inclusive slice [lo..hi] in an array.
/// @note constexpr for fixed-size arrays.
template <std::size_t N>
constexpr auto medianSortedSlice(const std::array<HighPrecisionFloat, N>& a,
                                 std::size_t lo, std::size_t hi) -> HighPrecisionFloat {
    const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
    if (len == 0) {
        return 0.0L;
    } else if (len & 1U) { // odd
        return a[lo + len / 2];
    } else {               // even
        const std::size_t midHi = lo + len / 2;
        const std::size_t midLo = midHi - 1;
        return (a[midLo] + a[midHi]) / 2.0L;
    }
}

/// @brief Median of an already-sorted whole array.
/// @note constexpr for fixed-size arrays.
template <std::size_t N>
constexpr auto medianSorted(const std::array<HighPrecisionFloat, N>& a) -> HighPrecisionFloat {
    if constexpr (N == 0) {
        return 0.0L;
    } else if constexpr (N % 2 == 1) {
        return a[N / 2];
    } else {
        return (a[N / 2 - 1] + a[N / 2]) / 2.0L;
    }
}

} // namespace detail


/// @brief Median of a (possibly unsorted) range. Materializes/sorts when needed.
/// @note Works for std::array (constexpr path) and for generic ranges (materializes to vector).
template <class R>
    requires num::NumberRange<R>
inline auto median(const R& r) -> HighPrecisionFloat {
    // If fixed-size array with constexpr path, prefer array overload:
    if constexpr (std::ranges::sized_range<R> && std::ranges::random_access_range<R> &&
                  std::is_array_v<std::remove_reference_t<R>>) {
        // fallback — but general code below handles arrays too, so this branch is optional.
    }

    // Materialize to HPF (vector) and sort for general ranges
    std::vector<HighPrecisionFloat> hp;
    hp.reserve(static_cast<std::size_t>(std::ranges::distance(r)));
    for (auto&& x : r) hp.push_back(toHPF(x));
    if (hp.empty()) return 0.0L;
    std::ranges::sort(hp);
    const auto n = hp.size();
    if (n % 2 == 1) return hp[n / 2];
    return (hp[n/2 - 1] + hp[n/2]) / 2.0L;
}

/// @brief Median of a sorted *array* or a sorted vector.
/// @note Keep this name because tests call medianSorted on arrays and expect constexpr behavior.
template <std::size_t N>
constexpr auto medianSorted(const std::array<HighPrecisionFloat, N>& sorted) -> HighPrecisionFloat {
    return detail::medianSorted(sorted);
}

/// @brieg Overload for sorted std::vector<HPF>
/// @param sorted sorted vector of HPF
/// @return median value (0.0L if empty)
inline auto medianSorted(const std::vector<HighPrecisionFloat>& sorted) -> HighPrecisionFloat {
    if (sorted.empty()) return 0.0L;
    const auto n = sorted.size();
    if (n % 2 == 1) return sorted[n / 2];
    return (sorted[n/2 - 1] + sorted[n/2]) / 2.0L;
}

/// @brief Tukey hinges on an already-sorted array<HPF, N>.
/// @note Median element is included in both halves when N is odd.
template <std::size_t N>
constexpr auto quartilesSorted(const std::array<HighPrecisionFloat, N>& sorted) -> QuartileSummary {
    if constexpr (N == 0) {
        return {};
    } else {
        const HighPrecisionFloat med = detail::medianSorted(sorted);

        std::size_t loL = 0, loH, hiL, hiH = N - 1;
        if constexpr (N % 2 == 1) {
            const std::size_t mid = N / 2;
            loH = mid;   // [0 .. mid]
            hiL = mid;   // [mid .. N-1]
        } else {
            loH = N / 2 - 1; // [0 .. N/2 - 1]
            hiL = N / 2;     // [N/2 .. N-1]
        }

        const HighPrecisionFloat q1 = detail::medianSortedSlice(sorted, loL, loH);
        const HighPrecisionFloat q3 = detail::medianSortedSlice(sorted, hiL, hiH);
        return {q1, med, q3};
    }
}

/// @brief Quartiles for std::array<T, N> (no heap allocations).
/// @tparam T arithmetic or HighPrecisionFloat.
/// @note Converts to HPF, sorts a local array copy, then computes hinges.
template <class T, std::size_t N>
    requires (std::is_arithmetic_v<std::remove_cvref_t<T>> ||
              std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr auto quartiles(const std::array<T, N>& data) -> QuartileSummary {
    if constexpr (N == 0) {
        return {};
    } else {
        std::array<HighPrecisionFloat, N> hp{};
        for (std::size_t i = 0; i < N; ++i) hp[i] = toHPF(data[i]);
        std::ranges::sort(hp);
        return quartilesSorted(hp);
    }
}

/// @brief Range-generic “sorted view” adapter for quartiles.
/// @details Materializes to a local vector<HPF>, sorts, then computes hinges.
/// @note Keeps array path constexpr while supporting vectors/spans/etc.
template <class R>
    requires num::NumberRange<R>
inline auto quartiles(const R& r) -> QuartileSummary {
    // Materialize to HPF
    std::vector<HighPrecisionFloat> hp;
    hp.reserve(static_cast<std::size_t>(std::ranges::distance(r)));
    for (auto&& x : r) hp.push_back(toHPF(x));

    if (hp.empty()) return {};

    // Sort and compute hinges
    std::ranges::sort(hp);

    const auto n = hp.size();
    const auto medianOfSlice = [&](std::size_t lo, std::size_t hi) -> HighPrecisionFloat {
        const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
        if (len == 0) return 0.0L;
        if (len & 1U) {
            return hp[lo + len / 2];
        } else {
            const std::size_t midHi = lo + len / 2;
            const std::size_t midLo = midHi - 1;
            return (hp[midLo] + hp[midHi]) / 2.0L;
        }
    };

    HighPrecisionFloat med{};
    if (n % 2U == 1U) {
        med = hp[n / 2];
    } else {
        med = (hp[n / 2 - 1] + hp[n / 2]) / 2.0L;
    }

    std::size_t loL = 0, loH, hiL, hiH = n - 1;
    if (n % 2U == 1U) {
        const std::size_t mid = n / 2;
        loH = mid;   // [0 .. mid]
        hiL = mid;   // [mid .. n-1]
    } else {
        loH = n / 2 - 1; // [0 .. n/2 - 1]
        hiL = n / 2;     // [n/2 .. n-1]
    }

    const HighPrecisionFloat q1 = medianOfSlice(loL, loH);
    const HighPrecisionFloat q3 = medianOfSlice(hiL, hiH);
    return {q1, med, q3};
}

} // namespace mally::statlib
