#pragma once
#include "HighPrecisionFloat.hpp"

#include <ranges>
#include <algorithm>
#include <type_traits>

namespace mally::statlib::num {

// Concepts
template <class R>
concept NumberRange =
    std::ranges::input_range<R> &&
    std::convertible_to<std::ranges::range_value_t<R>, mally::statlib::HighPrecisionFloat>;

// Sum in HighPrecisionFloat
template <NumberRange R>
constexpr mally::statlib::HighPrecisionFloat sum(const R& r)
{
    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for (auto&& x : r) acc += mally::statlib::toHPF(x);
    return acc;
}

// Average (returns 0 for empty)
template <NumberRange R>
constexpr mally::statlib::HighPrecisionFloat average(const R& r)
{
    const auto n = std::ranges::distance(r);
    if (n == 0) return 0.0L;
    return sum(r) / static_cast<mally::statlib::HighPrecisionFloat>(n);
}

// Min/Max values (not iterators)
template <NumberRange R>
constexpr auto minmax_value(const R& r)
    -> std::pair<mally::statlib::HighPrecisionFloat, mally::statlib::HighPrecisionFloat>
{
    if (std::ranges::empty(r)) return {0.0L, 0.0L};
    auto [itMin, itMax] = std::ranges::minmax_element(r);
    return { mally::statlib::toHPF(*itMin), mally::statlib::toHPF(*itMax) };
}

// Sum of products Σ x_i*y_i (sizes must match)
template <NumberRange RX, NumberRange RY>
constexpr mally::statlib::HighPrecisionFloat sum_product(const RX& x, const RY& y)
{
    auto itx = std::ranges::begin(x);
    auto ity = std::ranges::begin(y);
    const auto ex = std::ranges::end(x);
    const auto ey = std::ranges::end(y);

    mally::statlib::HighPrecisionFloat acc = 0.0L;
    for ( ; itx != ex && ity != ey; ++itx, ++ity)
        acc += mally::statlib::toHPF(*itx) * mally::statlib::toHPF(*ity);
    return acc;
}

} // namespace mally::statlib::num
