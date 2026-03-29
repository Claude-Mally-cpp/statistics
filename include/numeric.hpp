/// @file numeric.hpp
/// @brief Numeric helper functions for statistics library.
#pragma once
#include "HighPrecisionFloat.hpp"

#include <algorithm>
#include <concepts>
#include <expected>
#include <iterator>
#include <ranges>
#include <utility>

namespace mally::statlib::num
{

/// @brief Concept for ranges whose values convert to HighPrecisionFloat.
template <class R>
concept NumberRange = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_value_t<R>, mally::statlib::HighPrecisionFloat>;

/// @brief Sum of a numeric range in HighPrecisionFloat.
/// @note O(n), single pass.
template <NumberRange R> constexpr auto sum(const R& range) -> mally::statlib::HighPrecisionFloat
{
    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for (auto&& val : range)
    {
        acc += mally::statlib::toHPF(val);
    }
    return acc;
}

/// @brief Arithmetic mean of a numeric range (0 for empty).
/// @note O(n); uses HighPrecisionFloat accumulation.
template <NumberRange R> constexpr auto average(const R& range) -> mally::statlib::HighPrecisionFloat
{
    const auto count = std::ranges::distance(range);
    if (count == 0)
    {
        return 0.0L;
    }
    return sum(range) / static_cast<mally::statlib::HighPrecisionFloat>(count);
}

/// @brief Min & max values of a range as HighPrecisionFloat.
/// @note Requires forward iteration; returns {0,0} for empty.
template <class R>
    requires NumberRange<R> && std::ranges::forward_range<R>
constexpr auto minMaxValue(const R& range) -> std::pair<mally::statlib::HighPrecisionFloat, mally::statlib::HighPrecisionFloat>
{
    if (std::ranges::empty(range))
    {
        return {0.0L, 0.0L};
    }
    auto [itMin, itMax] = std::ranges::minmax_element(range);
    return {mally::statlib::toHPF(*itMin), mally::statlib::toHPF(*itMax)};
}

/// @brief Σ xrange_i * yrange_i as HighPrecisionFloat, with size checking.
/// @returns HighPrecisionResult{sum} on success; unexpected on size mismatch.
/// @note Works with single-pass ranges; detects mismatch if one ends earlier.
template <NumberRange RX, NumberRange RY>
constexpr auto sumProduct(const RX& xrange, const RY& yrange) -> mally::statlib::HighPrecisionResult
{
    auto       itx  = std::ranges::begin(xrange);
    auto       ity  = std::ranges::begin(yrange);
    const auto endx = std::ranges::end(xrange);
    const auto endy = std::ranges::end(yrange);

    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        acc += mally::statlib::toHPF(*itx) * mally::statlib::toHPF(*ity);
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::string{"sumProduct: ranges have different lengths"});
    }
    return acc;
}

} // namespace mally::statlib::num
