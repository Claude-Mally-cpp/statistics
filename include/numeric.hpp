/// @file numeric.hpp
/// @brief Numeric helper functions for statistics library.
#pragma once

#include "CalculationFloat.hpp"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace mally::statlib::num
{

/// @brief Concept for ranges whose values convert to `CalculationFloat`.
/// @tparam R Candidate range type.
template <class R>
concept NumberRange = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_value_t<R>, mally::statlib::CalculationFloat>;

/// @brief Concept for forward-iterable ranges whose values convert to `CalculationFloat`.
/// @details Use this for algorithms that traverse the range more than once.
/// @tparam R Candidate range type.
template <class R>
concept ForwardNumberRange = NumberRange<R> && std::ranges::forward_range<R>;

/// @brief Sum of a numeric range in `CalculationFloat`.
/// @param range Input range of numeric values.
/// @note O(n), single pass.
/// @return Sum of all values in `range`.
template <NumberRange R> constexpr auto sum(const R& range) -> mally::statlib::CalculationFloat
{
    mally::statlib::CalculationFloat acc = 0.0;
    for (auto&& val : range)
    {
        acc += static_cast<mally::statlib::CalculationFloat>(val);
    }
    return acc;
}

/// @brief Arithmetic mean of a numeric range (0 for empty).
/// @param range Input range of numeric values.
/// @note O(n); uses `CalculationFloat` accumulation. Requires forward iteration (traverses twice).
/// @return Arithmetic mean of `range`, or `0.0` when the range is empty.
template <ForwardNumberRange R> constexpr auto average(const R& range) -> mally::statlib::CalculationFloat
{
    const auto count = std::ranges::distance(range);
    if (count == 0)
    {
        return 0.0;
    }
    return sum(range) / static_cast<mally::statlib::CalculationFloat>(count);
}

/// @brief Min & max values of a range as `CalculationFloat`.
/// @param range Input range of numeric values.
/// @note Requires forward iteration; returns `{0.0, 0.0}` for empty.
/// @return Pair `{min, max}` converted to `CalculationFloat`, or `{0.0, 0.0}` for an empty range.
template <class R>
    requires NumberRange<R> && std::ranges::forward_range<R>
constexpr auto minMaxValue(const R& range) -> std::pair<mally::statlib::CalculationFloat, mally::statlib::CalculationFloat>
{
    if (std::ranges::empty(range))
    {
        return {0.0, 0.0};
    }
    auto [itMin, itMax] = std::ranges::minmax_element(range);
    return {
        static_cast<mally::statlib::CalculationFloat>(*itMin),
        static_cast<mally::statlib::CalculationFloat>(*itMax),
    };
}

/// @brief Σ xrange_i * yrange_i as `CalculationFloat`, with size checking.
/// @param xrange Left-hand input range.
/// @param yrange Right-hand input range.
/// @returns `CalculationResult` on success; unexpected on size mismatch.
/// @note Works with single-pass ranges; detects mismatch if one ends earlier.
template <NumberRange RX, NumberRange RY> constexpr auto sumProduct(const RX& xrange, const RY& yrange) -> mally::statlib::CalculationResult
{
    auto       itx  = std::ranges::begin(xrange);
    auto       ity  = std::ranges::begin(yrange);
    const auto endx = std::ranges::end(xrange);
    const auto endy = std::ranges::end(yrange);

    mally::statlib::CalculationFloat acc = 0.0;
    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        acc += static_cast<mally::statlib::CalculationFloat>(*itx) * static_cast<mally::statlib::CalculationFloat>(*ity);
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::string{"sumProduct: ranges have different lengths"});
    }
    return acc;
}

} // namespace mally::statlib::num
