/// @file numeric.hpp
/// @brief Numeric helper functions for statistics library.
#pragma once
#include "HighPrecisionFloat.hpp"

#include <algorithm>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

namespace mally::statlib::num
{

/// @brief Concept for ranges whose values convert to HighPrecisionFloat.
template <class R>
concept NumberRange = std::ranges::input_range<R> &&
                      std::convertible_to<std::ranges::range_value_t<R>, mally::statlib::HighPrecisionFloat>;

/// @brief Sum of a numeric range in HighPrecisionFloat.
/// @note O(n), single pass.
template <NumberRange R> constexpr auto sum(const R& r) -> mally::statlib::HighPrecisionFloat
{
    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for (auto&& x : r)
        acc += mally::statlib::toHPF(x);
    return acc;
}

/// @brief Arithmetic mean of a numeric range (0 for empty).
/// @note O(n); uses HighPrecisionFloat accumulation.
template <NumberRange R> constexpr auto average(const R& r) -> mally::statlib::HighPrecisionFloat
{
    const auto n = std::ranges::distance(r);
    if (n == 0)
        return 0.0L;
    return sum(r) / static_cast<mally::statlib::HighPrecisionFloat>(n);
}

/// @brief Min & max values of a range as HighPrecisionFloat.
/// @note Requires forward iteration; returns {0,0} for empty.
template <class R>
    requires NumberRange<R> && std::ranges::forward_range<R>
constexpr auto minMaxValue(const R& r)
    -> std::pair<mally::statlib::HighPrecisionFloat, mally::statlib::HighPrecisionFloat>
{
    if (std::ranges::empty(r))
        return {0.0L, 0.0L};
    auto [itMin, itMax] = std::ranges::minmax_element(r);
    return {mally::statlib::toHPF(*itMin), mally::statlib::toHPF(*itMax)};
}

/// @brief Σ x_i * y_i as HighPrecisionFloat, with size checking.
/// @returns HighPrecisionResult{sum} on success; unexpected on size mismatch.
/// @note Works with single-pass ranges; detects mismatch if one ends earlier.
template <NumberRange RX, NumberRange RY>
constexpr auto sumProduct(const RX& x, const RY& y) -> mally::statlib::HighPrecisionResult
{
    auto       itx = std::ranges::begin(x);
    auto       ity = std::ranges::begin(y);
    const auto ex  = std::ranges::end(x);
    const auto ey  = std::ranges::end(y);

    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for (; itx != ex && ity != ey; ++itx, ++ity)
    {
        acc += mally::statlib::toHPF(*itx) * mally::statlib::toHPF(*ity);
    }

    if (itx != ex || ity != ey)
    {
        return std::unexpected(std::string{"sumProduct: ranges have different lengths"});
    }
    return acc;
}

} // namespace mally::statlib::num
