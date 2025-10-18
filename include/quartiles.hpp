/// @file quartiles.hpp
/// @brief Functions to compute quartiles (Q1, median, Q3) of numeric ranges.

#pragma once
#include "HighPrecisionFloat.hpp"
#include "numeric.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <span>
#include <type_traits>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702 4127)
#endif

namespace mally::statlib
{

/// @brief Q1/median/Q3 summary.
struct QuartileSummary
{
    HighPrecisionFloat q1{};
    HighPrecisionFloat median{};
    HighPrecisionFloat q3{};
};

namespace detail
{

/// @brief Median of an already-sorted inclusive slice [lo..hi] in an array.
template <typename T, std::size_t N>
constexpr auto medianSortedSlice(const std::array<T, N>& a, std::size_t lo, std::size_t hi) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");
    const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
    if (len == 0)
        return 0.0L;
    if (len & 1U)
        return toHPF(a[lo + len / 2]);
    const std::size_t midHi = lo + len / 2;
    const std::size_t midLo = midHi - 1;
    return (toHPF(a[midLo]) + toHPF(a[midHi])) / 2.0L;
}

/// @brief Median of an already-sorted whole array.
template <typename T, std::size_t N> constexpr auto medianSorted(const std::array<T, N>& a) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");

    if constexpr (N == 0)
    {
        return 0.0L;
    }
    else if constexpr (N % 2 == 1)
    {
        return toHPF(a[N / 2]);
    }
    else
    {
        return (toHPF(a[N / 2 - 1]) + toHPF(a[N / 2])) / 2.0L;
    }
}

/// @brief Maximum extent we'll allow for stack-backed std::array conversion.
inline constexpr std::size_t maxStackHPFArraySize = 256U;

/// @brief Convert std::array<T, N> to std::array<HighPrecisionFloat, N>.
template <typename T, std::size_t N>
    requires std::is_convertible_v<T, HighPrecisionFloat>
constexpr auto toHPFArray(const std::array<T, N>& arr) -> std::array<HighPrecisionFloat, N>
{
    std::array<HighPrecisionFloat, N> out{};
    for (std::size_t i = 0; i < N; ++i)
        out[i] = toHPF(arr[i]);
    return out;
}

/// @brief Convert std::span<T, N> with static extent to std::array<HighPrecisionFloat, N>.
template <typename T, std::size_t N>
    requires(N != std::dynamic_extent) && (N <= detail::maxStackHPFArraySize) &&
            std::is_convertible_v<T, HighPrecisionFloat>
inline auto toHPFArray(const std::span<T, N>& s) -> std::array<HighPrecisionFloat, N>
{
    std::array<HighPrecisionFloat, N> out{};
    for (std::size_t i = 0; i < N; ++i)
        out[i] = toHPF(s[i]);
    return out;
}

/// @brief Convert any dynamic (or unknown-extent) NumberRange to std::vector<HighPrecisionFloat>.
template <class R>
    requires num::NumberRange<R>
inline auto toHPFVector(const R& r) -> std::vector<HighPrecisionFloat>
{
    std::vector<HighPrecisionFloat> out;
    out.reserve(static_cast<std::size_t>(std::ranges::distance(r)));
    for (auto&& x : r)
        out.push_back(toHPF(x));
    return out;
}

} // namespace detail

/// @brief Median of a (possibly unsorted) range. Materializes/sorts when needed.
template <class R>
    requires num::NumberRange<R>
inline auto median(const R& r) -> HighPrecisionFloat
{
    std::vector<HighPrecisionFloat> hp;
    hp.reserve(static_cast<std::size_t>(std::ranges::distance(r)));
    for (auto&& x : r)
        hp.push_back(toHPF(x));
    if (hp.empty())
        return 0.0L;
    std::ranges::sort(hp);
    const auto n = hp.size();
    return (n & 1U) ? hp[n / 2] : (hp[n / 2 - 1] + hp[n / 2]) / 2.0L;
}

/// @brief Median of a sorted *array* or a sorted vector.
template <typename T, std::size_t N>
constexpr auto medianSortedArray(const std::array<T, N>& sorted) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");
    return detail::medianSorted(sorted);
}

/// @brief Overload for sorted std::span<HighPrecisionFloat>
inline auto medianSortedSpan(std::span<const HighPrecisionFloat> sorted) -> HighPrecisionFloat
{
    if (sorted.empty())
        return 0.0L;
    const auto n = sorted.size();
    return (n & 1U) ? sorted[n / 2] : (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0L;
}

/// @brief Tukey hinges on an already-sorted array<HPF, N>.
template <std::size_t N>
constexpr auto quartilesSorted(const std::array<HighPrecisionFloat, N>& sorted) -> QuartileSummary
{
    if constexpr (N == 0)
        return {};
    const HighPrecisionFloat med = detail::medianSorted(sorted);
    std::size_t              loL = 0, loH, hiL, hiH = N - 1;
    if constexpr (N % 2 == 1)
    {
        const std::size_t mid = N / 2;
        if constexpr (N == 3)
        {
            loH = mid;
            hiL = mid;
        }
        else
        {
            loH = mid - 1;
            hiL = mid + 1;
        }
    }
    else
    {
        loH = N / 2 - 1;
        hiL = N / 2;
    }
    const HighPrecisionFloat q1 = detail::medianSortedSlice(sorted, loL, loH);
    const HighPrecisionFloat q3 = detail::medianSortedSlice(sorted, hiL, hiH);
    return {q1, med, q3};
}

/// @brief Quartiles for std::array<T, N> (no heap allocations).
template <class T, std::size_t N>
    requires(std::is_arithmetic_v<std::remove_cvref_t<T>> || std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr auto quartiles(const std::array<T, N>& data) -> QuartileSummary
{
    if constexpr (N == 0)
        return {};
    std::array<HighPrecisionFloat, N> hp{};
    for (std::size_t i = 0; i < N; ++i)
        hp[i] = toHPF(data[i]);
    std::ranges::sort(hp);
    return quartilesSorted(hp);
}

/// @brief Quartiles for ranges of numeric types (heap allocation if needed).
template <class R>
    requires num::NumberRange<R>
inline auto quartiles(const R& r) -> QuartileSummary
{
    std::vector<HighPrecisionFloat> hp;
    hp.reserve(static_cast<std::size_t>(std::ranges::distance(r)));
    for (auto&& x : r)
        hp.push_back(toHPF(x));
    if (hp.empty())
        return {};
    std::ranges::sort(hp);
    const auto n = hp.size();

    const auto medianOfSlice = [&](std::size_t lo, std::size_t hi) -> HighPrecisionFloat
    {
        const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
        if (len == 0)
            return 0.0L;
        if (len & 1U)
            return hp[lo + len / 2];
        const std::size_t midHi = lo + len / 2;
        const std::size_t midLo = midHi - 1;
        return (hp[midLo] + hp[midHi]) / 2.0L;
    };

    HighPrecisionFloat med = (n & 1U) ? hp[n / 2] : (hp[n / 2 - 1] + hp[n / 2]) / 2.0L;
    std::size_t        loL = 0, loH, hiL, hiH = n - 1;
    if (n & 1U)
    {
        const std::size_t mid = n / 2;
        if (n == 3U)
        {
            loH = mid;
            hiL = mid;
        }
        else
        {
            loH = mid - 1;
            hiL = mid + 1;
        }
    }
    else
    {
        loH = n / 2 - 1;
        hiL = n / 2;
    }
    const HighPrecisionFloat q1 = medianOfSlice(loL, loH);
    const HighPrecisionFloat q3 = medianOfSlice(hiL, hiH);
    return {q1, med, q3};
}

} // namespace mally::statlib

#ifdef _MSC_VER
#pragma warning(pop)
#endif
