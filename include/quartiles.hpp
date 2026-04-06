/// @file quartiles.hpp
/// @brief Functions to compute quartiles (Q1, median, Q3) of numeric ranges.

#pragma once
#include "HighPrecisionFloat.hpp"
#include "numeric.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
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
    HighPrecisionFloat q1{};     ///< @brief First quartile.
    HighPrecisionFloat median{}; ///< @brief Median.
    HighPrecisionFloat q3{};     ///< @brief Third quartile.
};

namespace detail
{

/// @brief Median of an already-sorted inclusive slice [low..high] in an array.
/// @tparam T Array element type convertible to `HighPrecisionFloat`.
/// @tparam N Array extent.
/// @param array Sorted input array.
/// @param low Inclusive lower index.
/// @param high Inclusive upper index.
/// @return Median of the slice, or `0.0L` if the slice is empty.
template <typename T, std::size_t N>
constexpr auto medianSortedSlice(const std::array<T, N>& array, std::size_t low, std::size_t high) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");
    const std::size_t len = (high >= low) ? (high - low + 1) : 0;
    if (len == 0)
    {
        return 0.0L;
    }
    if (len & 1U)
    {
        return toHPF(array[low + (len / 2)]);
    }
    const std::size_t midHi = low + (len / 2);
    const std::size_t midLo = midHi - 1;
    return (toHPF(array[midLo]) + toHPF(array[midHi])) / 2.0L;
}

/// @brief Median of an already-sorted whole array.
/// @tparam T Array element type convertible to `HighPrecisionFloat`.
/// @tparam N Array extent.
/// @param array Sorted input array.
/// @return Median of `array`, or `0.0L` when `N == 0`.
template <typename T, std::size_t N> constexpr auto medianSorted(const std::array<T, N>& array) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");

    if constexpr (N == 0)
    {
        return 0.0L;
    }
    else if constexpr (N % 2 == 1)
    {
        return toHPF(array[N / 2]);
    }
    else
    {
        return (toHPF(array[(N / 2) - 1]) + toHPF(array[N / 2])) / 2.0L;
    }
}

/// @brief Maximum extent we'll allow for stack-backed std::array conversion.
inline constexpr std::size_t maxStackHPFArraySize = 256U;

/// @brief Convert std::array<T, N> to std::array<HighPrecisionFloat, N>.
/// @tparam T Array element type convertible to `HighPrecisionFloat`.
/// @tparam N Array extent.
/// @param arr Input array.
/// @return Array converted element-wise to `HighPrecisionFloat`.
template <typename T, std::size_t N>
    requires std::is_convertible_v<T, HighPrecisionFloat>
constexpr auto toHPFArray(const std::array<T, N>& arr) -> std::array<HighPrecisionFloat, N>
{
    std::array<HighPrecisionFloat, N> out{};
    for (std::size_t i = 0; i < N; ++i)
    {
        out[i] = toHPF(arr[i]);
    }
    return out;
}

/// @brief Convert std::span<T, N> with static extent to std::array<HighPrecisionFloat, N>.
/// @tparam T Span element type convertible to `HighPrecisionFloat`.
/// @tparam N Static extent, limited by `maxStackHPFArraySize`.
/// @param spn Input span.
/// @return Array converted element-wise to `HighPrecisionFloat`.
template <typename T, std::size_t N>
    requires((N <= detail::maxStackHPFArraySize) && std::is_convertible_v<T, HighPrecisionFloat>)
inline auto toHPFArray(const std::span<T, N>& spn) -> std::array<HighPrecisionFloat, N>
{
    std::array<HighPrecisionFloat, N> out{};
    for (std::size_t i = 0; i < N; ++i)
    {
        out[i] = toHPF(spn[i]);
    }
    return out;
}

/// @brief Convert any dynamic (or unknown-extent) NumberRange to std::vector<HighPrecisionFloat>.
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Vector containing all values converted to `HighPrecisionFloat`.
template <class R>
    requires num::NumberRange<R>
inline auto toHPFVector(const R& range) -> std::vector<HighPrecisionFloat>
{
    std::vector<HighPrecisionFloat> out;
    if constexpr (std::ranges::sized_range<R>)
    {
        out.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }
    for (auto&& value : range)
    {
        out.push_back(toHPF(value));
    }
    return out;
}

} // namespace detail

/// @brief Median of a (possibly unsorted) range. Materializes/sorts when needed.
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Median of `range`, or `0.0L` if the range is empty.
template <class R>
    requires num::NumberRange<R>
inline auto median(const R& range) -> HighPrecisionFloat
{
    std::vector<HighPrecisionFloat> hpVector = detail::toHPFVector(range);
    if (hpVector.empty())
    {
        return 0.0L;
    }
    std::ranges::sort(hpVector);
    const auto count = hpVector.size();
    return (count & 1U) ? hpVector[count / 2] : (hpVector[(count / 2) - 1] + hpVector[count / 2]) / 2.0L;
}

/// @brief Median of an already-sorted array.
/// @tparam T Array element type convertible to `HighPrecisionFloat`.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Median of `sorted`.
template <typename T, std::size_t N> constexpr auto medianSortedArray(const std::array<T, N>& sorted) -> HighPrecisionFloat
{
    static_assert(std::is_convertible_v<T, HighPrecisionFloat>, "array element not convertible to HighPrecisionFloat");
    return detail::medianSorted(sorted);
}

/// @brief Median of an already-sorted `std::span<HighPrecisionFloat>`.
/// @param sorted Sorted input span.
/// @return Median of `sorted`, or `0.0L` if the span is empty.
inline auto medianSortedSpan(std::span<const HighPrecisionFloat> sorted) -> HighPrecisionFloat
{
    if (sorted.empty())
    {
        return 0.0L;
    }
    const auto count = sorted.size();
    return ((count & 1U) != 0U) ? sorted[count / 2] : (sorted[(count / 2) - 1] + sorted[count / 2]) / 2.0L;
}

/// @brief Compute Tukey hinges for an already-sorted span of HighPrecisionFloat.
/// @param sorted Sorted span of high-precision values.
/// @return Quartile summary containing Q1, median, and Q3, or zero-initialized if the span is empty.
/// @note Applies the same exclusive-median Tukey hinge convention as `quartilesSorted`, including
///       the special-case for n==3.
inline auto quartilesFromSortedSpan(std::span<const HighPrecisionFloat> sorted) -> QuartileSummary
{
    if (sorted.empty())
    {
        return {};
    }

    const auto count = sorted.size();

    const HighPrecisionFloat med = medianSortedSpan(sorted);

    const std::size_t loL{};
    std::size_t       loH{};
    std::size_t       hiL{};
    const std::size_t hiH = count - 1;

    if (count & 1U)
    {
        const std::size_t mid = count / 2;
        if (count == 3U)
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
        loH = (count / 2) - 1;
        hiL = count / 2;
    }

    const auto medianOfSlice = [&](std::size_t low, std::size_t high) -> HighPrecisionFloat
    {
        const std::size_t len = (high < low) ? 0 : (high - low + 1);
        if (len == 0)
        {
            return 0.0L;
        }
        if (len & 1U)
        {
            return sorted[low + (len / 2)];
        }
        const std::size_t midHi = low + (len / 2);
        const std::size_t midLo = midHi - 1;
        return (sorted[midLo] + sorted[midHi]) / 2.0L;
    };

    return {.q1 = medianOfSlice(loL, loH), .median = med, .q3 = medianOfSlice(hiL, hiH)};
}

/// @brief Compute Tukey hinges for an already-sorted `std::array<HighPrecisionFloat, N>`.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Quartile summary containing Q1, median, and Q3.
template <std::size_t N> constexpr auto quartilesSorted(const std::array<HighPrecisionFloat, N>& sorted) -> QuartileSummary
{
    if constexpr (N == 0)
    {
        return {};
    }
    const HighPrecisionFloat med = detail::medianSorted(sorted);
    const std::size_t        loL = 0;
    std::size_t              loH{};
    std::size_t              hiL{};
    const std::size_t        hiH = N - 1;
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
        loH = (N / 2) - 1;
        hiL = N / 2;
    }
    const HighPrecisionFloat quart1 = detail::medianSortedSlice(sorted, loL, loH);
    const HighPrecisionFloat quart3 = detail::medianSortedSlice(sorted, hiL, hiH);
    return {.q1 = quart1, .median = med, .q3 = quart3};
}

/// @brief Quartiles for std::array<T, N> (no heap allocations).
/// @tparam T Arithmetic or `HighPrecisionFloat` element type.
/// @tparam N Array extent.
/// @param data Input array.
/// @return Quartile summary for `data`.
template <class T, std::size_t N>
    requires(std::is_arithmetic_v<std::remove_cvref_t<T>> || std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr auto quartiles(const std::array<T, N>& data) -> QuartileSummary
{
    if constexpr (N == 0)
    {
        return {};
    }
    std::array<HighPrecisionFloat, N> hpArray{};
    for (std::size_t i = 0; i < N; ++i)
    {
        hpArray[i] = toHPF(data[i]);
    }
    std::ranges::sort(hpArray);
    return quartilesSorted(hpArray);
}

/// @brief Quartiles for ranges of numeric types (heap allocation if needed).
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Quartile summary for `range`, or a zero-initialized summary if the range is empty.
template <class R>
    requires num::NumberRange<R>
inline auto quartiles(const R& range) -> QuartileSummary
{
    std::vector<HighPrecisionFloat> hpVector = detail::toHPFVector(range);
    if (hpVector.empty())
    {
        return {};
    }
    std::ranges::sort(hpVector);
    return quartilesFromSortedSpan(hpVector);
}

} // namespace mally::statlib

#ifdef _MSC_VER
#pragma warning(pop)
#endif
