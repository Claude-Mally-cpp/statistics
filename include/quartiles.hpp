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
template <class T> struct QuartileSummary
{
    T q1{};     ///< @brief First quartile.
    T median{}; ///< @brief Median.
    T q3{};     ///< @brief Third quartile.
};

namespace detail
{

template <class T> using QuartileCalculationFloat = CalculationFloat<T>;
template <class T> using QuartilePublicResultType = PublicResultType<T>;

/// @brief Median of an already-sorted inclusive slice [low..high] in an array.
/// @tparam T Array element type.
/// @tparam N Array extent.
/// @param array Sorted input array at calculation precision.
/// @param low Inclusive lower index.
/// @param high Inclusive upper index.
/// @return Median of the slice, or `0.0` if the slice is empty.
template <typename T, std::size_t N>
constexpr auto medianSortedSlice(const std::array<QuartileCalculationFloat<T>, N>& array, std::size_t low, std::size_t high)
    -> QuartilePublicResultType<T>
{
    const std::size_t len = (high >= low) ? (high - low + 1) : 0;
    if (len == 0)
    {
        return static_cast<QuartilePublicResultType<T>>(0.0);
    }
    if (len & 1U)
    {
        return static_cast<QuartilePublicResultType<T>>(array[low + (len / 2)]);
    }
    const std::size_t midHi = low + (len / 2);
    const std::size_t midLo = midHi - 1;
    return static_cast<QuartilePublicResultType<T>>((array[midLo] + array[midHi]) / 2.0);
}

/// @brief Median of an already-sorted whole array.
/// @tparam T Array element type.
/// @tparam N Array extent.
/// @param array Sorted input array at calculation precision.
/// @return Median of `array`, or `0.0` when `N == 0`.
template <typename T, std::size_t N>
constexpr auto medianSorted(const std::array<QuartileCalculationFloat<T>, N>& array) -> QuartilePublicResultType<T>
{
    if constexpr (N == 0)
    {
        return static_cast<QuartilePublicResultType<T>>(0.0);
    }
    else if constexpr (N % 2 == 1)
    {
        return static_cast<QuartilePublicResultType<T>>(array[N / 2]);
    }
    else
    {
        return static_cast<QuartilePublicResultType<T>>((array[(N / 2) - 1] + array[N / 2]) / 2.0);
    }
}

/// @brief Convert any dynamic (or unknown-extent) NumberRange to a calculation-precision vector.
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Vector containing all values converted to the internal calculation type for the input.
template <class R>
    requires num::NumberRange<R>
inline auto materializeCalculationVector(const R& range) -> std::vector<num::RangeCalculationFloat<R>>
{
    std::vector<num::RangeCalculationFloat<R>> out;
    if constexpr (std::ranges::sized_range<R>)
    {
        out.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }
    for (auto&& value : range)
    {
        out.push_back(static_cast<num::RangeCalculationFloat<R>>(value));
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
inline auto median(const R& range) -> num::RangePublicResultType<R>
{
    std::vector<num::RangeCalculationFloat<R>> sortedValues = detail::materializeCalculationVector(range);
    if (sortedValues.empty())
    {
        return static_cast<num::RangePublicResultType<R>>(0.0);
    }
    std::ranges::sort(sortedValues);
    const auto count = sortedValues.size();
    const auto value = (count & 1U) ? sortedValues[count / 2] : (sortedValues[(count / 2) - 1] + sortedValues[count / 2]) / 2.0;
    return static_cast<num::RangePublicResultType<R>>(value);
}

/// @brief Median of an already-sorted array.
/// @tparam T Array element type.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Median of `sorted`.
template <typename T, std::size_t N> constexpr auto medianSortedArray(const std::array<T, N>& sorted) -> PublicResultType<T>
{
    std::array<CalculationFloat<T>, N> sortedValues{};
    for (std::size_t i = 0; i < N; ++i)
    {
        sortedValues[i] = static_cast<CalculationFloat<T>>(sorted[i]);
    }
    return detail::medianSorted<T>(sortedValues);
}

/// @brief Median of an already-sorted span of calculation-precision values.
/// @tparam T Original input value type driving public and calculation types.
/// @param sorted Sorted input span.
/// @return Median of `sorted`, or `0.0` if the span is empty.
template <class T> inline auto medianSortedSpan(std::span<const CalculationFloat<T>> sorted) -> PublicResultType<T>
{
    if (sorted.empty())
    {
        return static_cast<PublicResultType<T>>(0.0);
    }
    const auto count = sorted.size();
    const auto value = ((count & 1U) != 0U) ? sorted[count / 2] : (sorted[(count / 2) - 1] + sorted[count / 2]) / 2.0;
    return static_cast<PublicResultType<T>>(value);
}

/// @brief Compute Tukey hinges for an already-sorted span of calculation-precision values.
/// @tparam T Original input value type driving public and calculation types.
/// @param sorted Sorted span of values.
/// @return Quartile summary containing Q1, median, and Q3, or zero-initialized if the span is empty.
/// @note Applies the same exclusive-median Tukey hinge convention as `quartilesSorted`, including
///       the special-case for n==3.
template <class T> inline auto quartilesFromSortedSpan(std::span<const CalculationFloat<T>> sorted) -> QuartileSummary<PublicResultType<T>>
{
    if (sorted.empty())
    {
        return {};
    }

    const auto count = sorted.size();

    const PublicResultType<T> med = medianSortedSpan<T>(sorted);

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

    const auto medianOfSlice = [&](std::size_t low, std::size_t high) -> PublicResultType<T>
    {
        const std::size_t len = (high < low) ? 0 : (high - low + 1);
        if (len == 0)
        {
            return static_cast<PublicResultType<T>>(0.0);
        }
        if (len & 1U)
        {
            return static_cast<PublicResultType<T>>(sorted[low + (len / 2)]);
        }
        const std::size_t midHi = low + (len / 2);
        const std::size_t midLo = midHi - 1;
        return static_cast<PublicResultType<T>>((sorted[midLo] + sorted[midHi]) / 2.0);
    };

    return {.q1 = medianOfSlice(loL, loH), .median = med, .q3 = medianOfSlice(hiL, hiH)};
}

/// @brief Compute Tukey hinges for an already-sorted array at calculation precision.
/// @tparam T Original input value type driving public and calculation types.
/// @tparam N Array extent.
/// @param sorted Sorted input array.
/// @return Quartile summary containing Q1, median, and Q3.
template <class T, std::size_t N>
constexpr auto quartilesSorted(const std::array<CalculationFloat<T>, N>& sorted) -> QuartileSummary<PublicResultType<T>>
{
    if constexpr (N == 0)
    {
        return {};
    }
    const PublicResultType<T> med = detail::medianSorted<T>(sorted);
    const std::size_t         loL = 0;
    std::size_t               loH{};
    std::size_t               hiL{};
    const std::size_t         hiH = N - 1;
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
    const PublicResultType<T> quart1 = detail::medianSortedSlice<T>(sorted, loL, loH);
    const PublicResultType<T> quart3 = detail::medianSortedSlice<T>(sorted, hiL, hiH);
    return {.q1 = quart1, .median = med, .q3 = quart3};
}

/// @brief Quartiles for std::array<T, N> (no heap allocations).
/// @tparam T Arithmetic element type.
/// @tparam N Array extent.
/// @param data Input array.
/// @return Quartile summary for `data`.
template <class T, std::size_t N>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr auto quartiles(const std::array<T, N>& data) -> QuartileSummary<PublicResultType<T>>
{
    if constexpr (N == 0)
    {
        return {};
    }
    std::array<CalculationFloat<T>, N> sortedValues{};
    for (std::size_t i = 0; i < N; ++i)
    {
        sortedValues[i] = static_cast<CalculationFloat<T>>(data[i]);
    }
    std::ranges::sort(sortedValues);
    return quartilesSorted<T>(sortedValues);
}

/// @brief Quartiles for ranges of numeric types (heap allocation if needed).
/// @tparam R Numeric input range type.
/// @param range Input range.
/// @return Quartile summary for `range`, or a zero-initialized summary if the range is empty.
template <class R>
    requires num::NumberRange<R>
inline auto quartiles(const R& range) -> QuartileSummary<num::RangePublicResultType<R>>
{
    std::vector<num::RangeCalculationFloat<R>> sortedValues = detail::materializeCalculationVector(range);
    if (sortedValues.empty())
    {
        return {};
    }
    std::ranges::sort(sortedValues);
    return quartilesFromSortedSpan<num::RangeValueType<R>>(sortedValues);
}

} // namespace mally::statlib

#ifdef _MSC_VER
#pragma warning(pop)
#endif
