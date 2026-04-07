/// @file numeric.hpp
/// @brief Numeric helper functions for statistics library.
#pragma once

#include <algorithm>
#include <concepts>
#include <expected>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace mally::statlib::num
{

/// @brief Concept for ranges whose values convert to `long double`.
/// @tparam R Candidate range type.
template <class R>
concept NumberRange = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_value_t<R>, long double>;

/// @brief Concept for forward-iterable ranges whose values convert to `long double`.
/// @details Use this for algorithms that traverse the range more than once.
/// @tparam R Candidate range type.
template <class R>
concept ForwardNumberRange = NumberRange<R> && std::ranges::forward_range<R>;

/// @brief Sum of a numeric range in `long double`.
/// @param range Input range of numeric values.
/// @note O(n), single pass.
/// @return Sum of all values in `range`.
template <NumberRange R> constexpr auto sum(const R& range) -> long double
{
    long double acc = 0.0L;
    for (auto&& val : range)
    {
        acc += static_cast<long double>(val);
    }
    return acc;
}

/// @brief Arithmetic mean of a numeric range (0 for empty).
/// @param range Input range of numeric values.
/// @note O(n); uses `long double` accumulation. Requires forward iteration (traverses twice).
/// @return Arithmetic mean of `range`, or `0.0L` when the range is empty.
template <ForwardNumberRange R> constexpr auto average(const R& range) -> long double
{
    const auto count = std::ranges::distance(range);
    if (count == 0)
    {
        return 0.0L;
    }
    return sum(range) / static_cast<long double>(count);
}

/// @brief Min & max values of a range as `long double`.
/// @param range Input range of numeric values.
/// @note Requires forward iteration; returns {0,0} for empty.
/// @return Pair `{min, max}` converted to `long double`, or `{0.0L, 0.0L}` for an empty range.
template <class R>
    requires NumberRange<R> && std::ranges::forward_range<R>
constexpr auto minMaxValue(const R& range) -> std::pair<long double, long double>
{
    if (std::ranges::empty(range))
    {
        return {0.0L, 0.0L};
    }
    auto [itMin, itMax] = std::ranges::minmax_element(range);
    return {static_cast<long double>(*itMin), static_cast<long double>(*itMax)};
}

/// @brief Σ xrange_i * yrange_i as `long double`, with size checking.
/// @param xrange Left-hand input range.
/// @param yrange Right-hand input range.
/// @returns `std::expected<long double, std::string>` on success; unexpected on size mismatch.
/// @note Works with single-pass ranges; detects mismatch if one ends earlier.
template <NumberRange RX, NumberRange RY>
constexpr auto sumProduct(const RX& xrange, const RY& yrange) -> std::expected<long double, std::string>
{
    auto       itx  = std::ranges::begin(xrange);
    auto       ity  = std::ranges::begin(yrange);
    const auto endx = std::ranges::end(xrange);
    const auto endy = std::ranges::end(yrange);

    long double acc = 0.0L;
    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        acc += static_cast<long double>(*itx) * static_cast<long double>(*ity);
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::string{"sumProduct: ranges have different lengths"});
    }
    return acc;
}

} // namespace mally::statlib::num
