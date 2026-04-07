/// @file numeric.hpp
/// @brief Numeric helper functions for statistics library.
#pragma once

#include "CalculationFloat.hpp"

#include <algorithm>
#include <expected>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace mally::statlib::num
{

template <class R> using RangeValueType = std::remove_cvref_t<std::ranges::range_value_t<R>>;

template <class R> using RangePublicResultType = mally::statlib::PublicResultType<RangeValueType<R>>;

template <class R> using RangeCalculationFloat = mally::statlib::CalculationFloat<RangeValueType<R>>;

template <class RX, class RY> using PairRangeValueType = std::common_type_t<RangeValueType<RX>, RangeValueType<RY>>;

template <class RX, class RY> using PairPublicResultType = mally::statlib::PublicResultType<PairRangeValueType<RX, RY>>;

template <class RX, class RY> using PairCalculationFloat = mally::statlib::CalculationFloat<PairRangeValueType<RX, RY>>;

/// @brief Concept for ranges whose values are arithmetic.
/// @tparam R Candidate range type.
template <class R>
concept NumberRange = std::ranges::input_range<R> && std::is_arithmetic_v<RangeValueType<R>>;

/// @brief Concept for forward-iterable ranges whose values are arithmetic.
/// @details Use this for algorithms that traverse the range more than once.
/// @tparam R Candidate range type.
template <class R>
concept ForwardNumberRange = NumberRange<R> && std::ranges::forward_range<R>;

/// @brief Sum of a numeric range.
/// @param range Input range of numeric values.
/// @note O(n), single pass.
/// @return Sum of all values in `range`, converted to the public result type for that input.
template <NumberRange R> constexpr auto sum(const R& range) -> RangePublicResultType<R>
{
    RangeCalculationFloat<R> acc = 0.0;
    for (auto&& val : range)
    {
        acc += static_cast<RangeCalculationFloat<R>>(val);
    }
    return static_cast<RangePublicResultType<R>>(acc);
}

/// @brief Arithmetic mean of a numeric range (0 for empty).
/// @param range Input range of numeric values.
/// @note O(n); uses widened internal accumulation and returns the public result type for the input.
/// @return Arithmetic mean of `range`, or `0.0` when the range is empty.
template <ForwardNumberRange R> constexpr auto average(const R& range) -> RangePublicResultType<R>
{
    const auto count = std::ranges::distance(range);
    if (count == 0)
    {
        return static_cast<RangePublicResultType<R>>(0.0);
    }
    RangeCalculationFloat<R> total = 0.0;
    for (auto&& val : range)
    {
        total += static_cast<RangeCalculationFloat<R>>(val);
    }
    return static_cast<RangePublicResultType<R>>(total / static_cast<RangeCalculationFloat<R>>(count));
}

/// @brief Min & max values of a range.
/// @param range Input range of numeric values.
/// @note Requires forward iteration; returns `{0.0, 0.0}` for empty.
/// @return Pair `{min, max}` converted to the public result type for the input.
template <class R>
    requires ForwardNumberRange<R>
constexpr auto minMaxValue(const R& range) -> std::pair<RangePublicResultType<R>, RangePublicResultType<R>>
{
    if (std::ranges::empty(range))
    {
        return {
            static_cast<RangePublicResultType<R>>(0.0),
            static_cast<RangePublicResultType<R>>(0.0),
        };
    }
    auto [itMin, itMax] = std::ranges::minmax_element(range);
    return {
        static_cast<RangePublicResultType<R>>(*itMin),
        static_cast<RangePublicResultType<R>>(*itMax),
    };
}

/// @brief Σ xrange_i * yrange_i with size checking.
/// @param xrange Left-hand input range.
/// @param yrange Right-hand input range.
/// @returns Public result type based on the common input type, or unexpected on size mismatch.
/// @note Works with single-pass ranges; detects mismatch if one ends earlier.
template <NumberRange RX, NumberRange RY>
constexpr auto sumProduct(const RX& xrange, const RY& yrange) -> std::expected<PairPublicResultType<RX, RY>, std::string>
{
    auto       itx  = std::ranges::begin(xrange);
    auto       ity  = std::ranges::begin(yrange);
    const auto endx = std::ranges::end(xrange);
    const auto endy = std::ranges::end(yrange);

    PairCalculationFloat<RX, RY> acc = 0.0;
    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        acc += static_cast<PairCalculationFloat<RX, RY>>(*itx) * static_cast<PairCalculationFloat<RX, RY>>(*ity);
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::string{"sumProduct: ranges have different lengths"});
    }
    return static_cast<PairPublicResultType<RX, RY>>(acc);
}

} // namespace mally::statlib::num
