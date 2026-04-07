/// @file quartiles.hpp
/// @brief Functions to compute quartiles (Q1, median, Q3) of numeric ranges.

#pragma once
#include "CalculationFloat.hpp"
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
    CalculationFloat q1{};     ///< @brief First quartile.
    CalculationFloat median{}; ///< @brief Median.
    CalculationFloat q3{};     ///< @brief Third quartile.
};

namespace detail
{

/// @brief Median of an already-sorted inclusive slice [low..high] in an array.
/// @tparam T Array element type convertible to `CalculationFloat`.
/// @tparam N Array extent.
/// @param array Sorted input array.
/// @param low Inclusive lower index.
/// @param high Inclusive upper index.
/// @return Median of the slice, or `0.0` if the slice is empty.
template <typename T, std::size_t N>
constexpr auto medianSortedSlice(const std::array<T, N>& array, std::size_t low, std::size_t high) -> CalculationFloat
{
    static_assert(std::is_convertible_v<T, CalculationFloat>, "array element not convertible to CalculationFloat");
    const std::size_t len = (high >= low) ? (high - low + 1) : 0;
    if (len == 0)
    {
        return 0.0;
    }
    if (len & 1U)
    {
        return static_cast<CalculationFloat>(array[low + (len / 2)]);
    }
    const std::size_t midHi = low + (len / 2);
    const std::size_t midLo = midHi - 1;
    return (static_cast<CalculationFloat>(array[midLo]) + static_cast<CalculationFloat>(array[midHi])) / 2.0;
}

/// @brief Median of an already-sorted whole array.
/// @tparam T Array element type convertible to `CalculationFloat`.
/// @tparam N Array extent.
/// @param array Sorted input array.
/// @return Median of `array`, or `0.0` when `N == 0`.
template <typename T, std::size_t N> constexpr auto medianSorted(const std::array<T, N>& array) -> CalculationFloat
{
    static_assert(std::is_convertible_v<T, CalculationFloat>, "array element not convertible to CalculationFloat");

    if constexpr (N == 0)
    {
        return 0.0;
    }
    else if constexpr (N % 2 == 1)
    {
        return static_cast<CalculationFloat>(array[N / 2]);
    }
    else
    {
        return (static_cast<CalculationFloat>(array[(N / 2) - 1]) + static_cast<CalculationFloat>(array[N / 2])) / 2.0;
    }
}

/// @brief Convert any dynamic (or unknown-extent) NumberRange to `std::vector<CalculationFloat>`.
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Vector containing all values converted to `CalculationFloat`.
template <class R>
    requires num::NumberRange<R>
inline auto materializeCalculationVector(const R& range) -> std::vector<CalculationFloat>
{
    std::vector<CalculationFloat> out;
    if constexpr (std::ranges::sized_range<R>)
    {
        out.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }
    for (auto&& value : range)
    {
        out.push_back(static_cast<CalculationFloat>(value));
    }
    return out;
}

} // namespace detail

/// @brief Median of a (possibly unsorted) range. Materializes/sorts when needed.
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Median of `range`, or `0.0` if the range is empty.
template <class R>
    requires num::NumberRange<R>
inline auto median(const R& range) -> CalculationFloat
{
    std::vector<CalculationFloat> sortedValues = detail::materializeCalculationVector(range);
    if (sortedValues.empty())
    {
        return 0.0;
    }
    std::ranges::sort(sortedValues);
    const auto count = sortedValues.size();
    return (count & 1U) ? sortedValues[count / 2] : (sortedValues[(count / 2) - 1] + sortedValues[count / 2]) / 2.0;
}

/// @brief Median of an already-sorted array.
/// @tparam T Array element type convertible to `CalculationFloat`.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Median of `sorted`.
template <typename T, std::size_t N> constexpr auto medianSortedArray(const std::array<T, N>& sorted) -> CalculationFloat
{
    static_assert(std::is_convertible_v<T, CalculationFloat>, "array element not convertible to CalculationFloat");
    return detail::medianSorted(sorted);
}

/// @brief Median of an already-sorted `std::span<const CalculationFloat>`.
/// @param sorted Sorted input span.
/// @return Median of `sorted`, or `0.0` if the span is empty.
inline auto medianSortedSpan(std::span<const CalculationFloat> sorted) -> CalculationFloat
{
    if (sorted.empty())
    {
        return 0.0;
    }
    const auto count = sorted.size();
    return ((count & 1U) != 0U) ? sorted[count / 2] : (sorted[(count / 2) - 1] + sorted[count / 2]) / 2.0;
}

/// @brief Compute Tukey hinges for an already-sorted span of `CalculationFloat`.
/// @param sorted Sorted span of values.
/// @return Quartile summary containing Q1, median, and Q3, or zero-initialized if the span is empty.
/// @note Applies the same exclusive-median Tukey hinge convention as `quartilesSorted`, including
///       the special-case for n==3.
inline auto quartilesFromSortedSpan(std::span<const CalculationFloat> sorted) -> QuartileSummary
{
    if (sorted.empty())
    {
        return {};
    }

    const auto count = sorted.size();

    const CalculationFloat med = medianSortedSpan(sorted);

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

    const auto medianOfSlice = [&](std::size_t low, std::size_t high) -> CalculationFloat
    {
        const std::size_t len = (high < low) ? 0 : (high - low + 1);
        if (len == 0)
        {
            return 0.0;
        }
        if (len & 1U)
        {
            return sorted[low + (len / 2)];
        }
        const std::size_t midHi = low + (len / 2);
        const std::size_t midLo = midHi - 1;
        return (sorted[midLo] + sorted[midHi]) / 2.0;
    };

    return {.q1 = medianOfSlice(loL, loH), .median = med, .q3 = medianOfSlice(hiL, hiH)};
}

/// @brief Compute Tukey hinges for an already-sorted `std::array<CalculationFloat, N>`.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Quartile summary containing Q1, median, and Q3.
template <std::size_t N> constexpr auto quartilesSorted(const std::array<CalculationFloat, N>& sorted) -> QuartileSummary
{
    if constexpr (N == 0)
    {
        return {};
    }
    const CalculationFloat med = detail::medianSorted(sorted);
    const std::size_t      loL = 0;
    std::size_t            loH{};
    std::size_t            hiL{};
    const std::size_t      hiH = N - 1;
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
    const CalculationFloat quart1 = detail::medianSortedSlice(sorted, loL, loH);
    const CalculationFloat quart3 = detail::medianSortedSlice(sorted, hiL, hiH);
    return {.q1 = quart1, .median = med, .q3 = quart3};
}

/// @brief Quartiles for std::array<T, N> (no heap allocations).
/// @tparam T Arithmetic element type.
/// @tparam N Array extent.
/// @param data Input array.
/// @return Quartile summary for `data`.
template <class T, std::size_t N>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr auto quartiles(const std::array<T, N>& data) -> QuartileSummary
{
    if constexpr (N == 0)
    {
        return {};
    }
    std::array<CalculationFloat, N> sortedValues{};
    for (std::size_t i = 0; i < N; ++i)
    {
        sortedValues[i] = static_cast<CalculationFloat>(data[i]);
    }
    std::ranges::sort(sortedValues);
    return quartilesSorted(sortedValues);
}

/// @brief Quartiles for ranges of numeric types (heap allocation if needed).
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Quartile summary for `range`, or a zero-initialized summary if the range is empty.
template <class R>
    requires num::NumberRange<R>
inline auto quartiles(const R& range) -> QuartileSummary
{
    std::vector<CalculationFloat> sortedValues = detail::materializeCalculationVector(range);
    if (sortedValues.empty())
    {
        return {};
    }
    std::ranges::sort(sortedValues);
    return quartilesFromSortedSpan(sortedValues);
}

} // namespace mally::statlib

#ifdef _MSC_VER
#pragma warning(pop)
#endif
