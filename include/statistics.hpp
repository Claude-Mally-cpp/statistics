/// @file statistics.hpp : minimalist statistics library
/// @brief This file contains functions to compute statistics on range(s) of numbers.
/// @details Public result types follow the input value type, while internal calculation may widen.
/// @author Claude Mally
/// @date 2025-04-11
///
/// Notes and conventions
/// - Header location: this file is intended to be installed and used from the project's
///   "include" directory. When building, add the repository root include folder to
///   your compiler's include path and then include "statistics.hpp".
/// - Quartile convention: this library computes quartiles using the Tukey hinge style.
///   By default the implementation uses the exclusive-median variant (the median element
///   is excluded from both lower and upper halves when computing Q1 and Q3). For a
///   very small sample (size == 3) a small-sample special-case is applied and the
///   median is included in both halves to match common textbook examples. See
///   quartiles.hpp and the unit tests under test/test_statistics.cpp for details.
/// - Summary output: `summary(range)` returns min, Q1, median, mean, Q3 and max; mean
///   is computed with `average(range)` which returns 0 on empty ranges.
///
/// Example usage:
///   #include "statistics.hpp"
///   using namespace mally::statlib;
///   auto s = summary(myVector);
#pragma once

#include "CalculationFloat.hpp"
#include "numeric.hpp"
#include "quartiles.hpp"
#include "summaryStats.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <iterator>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <vector>

namespace mally::statlib
{

/// @brief Enable verbose debugging output to stderr (for development).
/// @note Disabled by default; enable manually when needed.
inline constexpr bool verboseDebugging = false;

using num::ForwardNumberRange;
using num::NumberRange;

/// @brief Summary for an std::array without allocations.
/// @tparam T Arithmetic element type.
/// @tparam N Array extent.
/// @param data Input array of numeric values.
/// @note Computes internally at `CalculationFloat<T>` precision, then converts to `SummaryStats<PublicResultType<T>>`.
/// @return Summary statistics for `data`.
template <class T, std::size_t N>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr auto summary(const std::array<T, N>& data) -> SummaryStats<PublicResultType<T>>
{
    SummaryStats<PublicResultType<T>> out{};
    out.count = N;

    if constexpr (N == 0)
    {
        return out;
    }
    else
    {
        // Materialize at calculation precision and accumulate the sum in one pass.
        std::array<CalculationFloat<T>, N> sortedValues{};
        CalculationFloat<T>                sumAcc = 0.0;
        for (std::size_t i = 0; i < N; ++i)
        {
            sortedValues[i] = static_cast<CalculationFloat<T>>(data[i]);
            sumAcc += sortedValues[i];
        }
        std::ranges::sort(sortedValues);

        // Min/max from sorted endpoints; mean from accumulated sum
        out.min  = static_cast<PublicResultType<T>>(sortedValues.front());
        out.max  = static_cast<PublicResultType<T>>(sortedValues.back());
        out.mean = static_cast<PublicResultType<T>>(sumAcc / static_cast<CalculationFloat<T>>(N));

        // Quartiles from sorted array
        const auto qSorted = quartilesSorted<T>(sortedValues);
        out.q1             = qSorted.q1;
        out.median         = qSorted.median;
        out.q3             = qSorted.q3;

        return out;
    }
}

/// @brief Summary for a generic numeric range (vectors, spans, views...).
/// @tparam R Numeric input range type.
/// @param range Input range of numeric values.
/// @details Materializes to a calculation-precision vector in one pass (accumulating the sum), sorts once,
///          then derives min/max from the sorted endpoints and quartiles via
///          quartilesFromSortedSpan(). This avoids repeated range traversals and redundant
///          heap allocations compared with calling individual helpers separately.
/// @note Requires forward iteration for the empty-check; the range body is single-pass.
/// @return Summary statistics for `range`, or a zero-initialized summary for an empty range.
template <class R>
    requires num::ForwardNumberRange<R>
constexpr auto summary(const R& range) -> SummaryStats<num::RangePublicResultType<R>>
{
    SummaryStats<num::RangePublicResultType<R>> out{};

    if (std::ranges::empty(range))
    {
        return out;
    }

    // One pass: materialize at calculation precision and accumulate the sum simultaneously.
    std::vector<num::RangeCalculationFloat<R>> sortedValues;
    if constexpr (std::ranges::sized_range<R>)
    {
        sortedValues.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }
    num::RangeCalculationFloat<R> sumAcc = 0.0;
    for (auto&& val : range)
    {
        const auto converted = static_cast<num::RangeCalculationFloat<R>>(val);
        sortedValues.push_back(converted);
        sumAcc += converted;
    }

    out.count = sortedValues.size();

    // Sort once; min and max are the endpoints of the sorted sequence
    std::ranges::sort(sortedValues);
    out.min  = static_cast<num::RangePublicResultType<R>>(sortedValues.front());
    out.max  = static_cast<num::RangePublicResultType<R>>(sortedValues.back());
    out.mean = static_cast<num::RangePublicResultType<R>>(sumAcc / static_cast<num::RangeCalculationFloat<R>>(out.count));

    // Reuse the sorted data for all three quartiles
    const auto q = quartilesFromSortedSpan<num::RangeValueType<R>>(sortedValues);
    out.q1       = q.q1;
    out.median   = q.median;
    out.q3       = q.q3;

    return out;
}

/// @brief Compute the product of a range of numbers.
/// @param range Input range of numbers.
/// @details Uses widened internal accumulation and converts back to the public result type.
/// @return Product of all values in `range`.
template <NumberRange R> constexpr auto product(const R& range) -> num::RangePublicResultType<R>
{
    const auto value = std::accumulate(std::ranges::begin(range),
                                       std::ranges::end(range),
                                       num::RangeCalculationFloat<R>{1.0},
                                       [](num::RangeCalculationFloat<R> acc, auto val) -> auto
                                       { return acc * static_cast<num::RangeCalculationFloat<R>>(val); });
    return static_cast<num::RangePublicResultType<R>>(value);
}

/// @brief Compute the geometric mean of a range of numbers.
/// @param range Input range of numbers.
/// @details Uses widened internal intermediates during the calculation.
/// @return Geometric mean of the values in `range`, or `0.0` for an empty range.
template <NumberRange R> auto geometricMean(const R& range) -> num::RangePublicResultType<R>
{
    auto        totalProduct = num::RangeCalculationFloat<R>{1.0};
    std::size_t count        = 0;

    for (const auto& val : range)
    {
        totalProduct *= static_cast<num::RangeCalculationFloat<R>>(val);
        ++count;
    }

    if (count == 0)
    {
        return static_cast<num::RangePublicResultType<R>>(0.0);
    }

    const auto value = std::pow(totalProduct,
                                num::RangeCalculationFloat<R>{1.0}
                                    / static_cast<num::RangeCalculationFloat<R>>(count));
    return static_cast<num::RangePublicResultType<R>>(value);
}

/// @brief Compute the sum of squares of a range of numbers.
/// @param range Input range of numbers.
/// @return Sum of squared values in `range`.
template <NumberRange R> constexpr auto sumSquared(const R& range) -> num::RangePublicResultType<R>
{
    const auto value = std::accumulate(std::ranges::begin(range),
                                       std::ranges::end(range),
                                       num::RangeCalculationFloat<R>{0.0},
                                       [](num::RangeCalculationFloat<R> acc, auto val) -> auto
                                       {
                                           const auto widenedValue = static_cast<num::RangeCalculationFloat<R>>(val);
                                           const auto valueSquared = widenedValue * widenedValue;
                                           return acc + valueSquared;
                                       });
    return static_cast<num::RangePublicResultType<R>>(value);
}

/// @brief Reusable part of the denominator of the correlation coefficient formula.
/// @tparam CalcT Internal calculation type.
/// @details This function computes either the x or the y denominator portion of
/// the correlation coefficient formula:
/// sqrt(n * sum(x^2) - (sum(x))^2) * sqrt(n * sum(y^2) - (sum(y))^2)
/// @param sum Sum of the elements in the range.
/// @param sumSquared Sum of squares of the elements in the range.
/// @param n Number of elements in the range.
/// @return `std::expected<CalcT, std::string>`
template <class CalcT> auto rawDeviationDenominatorPart(auto sum, auto sumSquared, std::size_t n) -> std::expected<CalcT, std::string>
{
    const auto n_calc          = static_cast<CalcT>(n);
    const auto sum_calc        = static_cast<CalcT>(sum);
    const auto sumSquared_calc = static_cast<CalcT>(sumSquared);

    const auto radicand = (n_calc * sumSquared_calc) - (sum_calc * sum_calc);
    if (radicand < 0)
    {
        return std::unexpected(std::format("{} * {} - {}^2={}", n, sumSquared, sum, radicand));
    }

    if constexpr (verboseDebugging)
    {
        println("rawDeviationDenominatorPart: n={} sum={} sumSquared={} radicand={}", n, sum, sumSquared, radicand);
    }

    return std::sqrt(radicand);
}

/// @brief Compute the Pearson correlation coefficient of two numeric ranges.
/// @param range_x First input range.
/// @param range_y Second input range.
/// @details Uses a single fused pass to accumulate n, sigma_x, sigma_y, sigma_x2, sigma_y2,
///          and sigma_xy simultaneously, avoiding the five separate traversals of the
///          original implementation.
/// @return Correlation coefficient on success, or an error if the inputs differ in size, have too few elements, or yield an invalid
/// denominator.
template <ForwardNumberRange RX, ForwardNumberRange RY>
auto correlationCoefficient(const RX& range_x, const RY& range_y) -> std::expected<num::PairPublicResultType<RX, RY>, std::string>
{
    // Fused single pass: count n and accumulate all five sums simultaneously
    num::PairCalculationFloat<RX, RY> sigma_x{}, sigma_y{}, sigma_x2{}, sigma_y2{}, sigma_xy{};
    std::size_t                       n{};

    auto       itx  = std::ranges::begin(range_x);
    auto       ity  = std::ranges::begin(range_y);
    const auto endx = std::ranges::end(range_x);
    const auto endy = std::ranges::end(range_y);

    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        const auto xi = static_cast<num::PairCalculationFloat<RX, RY>>(*itx);
        const auto yi = static_cast<num::PairCalculationFloat<RX, RY>>(*ity);
        sigma_x += xi;
        sigma_y += yi;
        sigma_x2 += xi * xi;
        sigma_y2 += yi * yi;
        sigma_xy += xi * yi;
        ++n;
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::format("correlationCoefficient: ranges have different lengths"));
    }

    if (n < 2)
    {
        return std::unexpected(std::format("not enough data points: n={}", n));
    }

    const auto count     = static_cast<num::PairCalculationFloat<RX, RY>>(n);
    const auto numerator = (count * sigma_xy) - (sigma_x * sigma_y);
    if constexpr (verboseDebugging)
    {
        println("count={} sigma_x={} sigma_y={} sigma_xy={} numerator={}", count, sigma_x, sigma_y, sigma_xy, numerator);
    }

    const auto denominator_x = rawDeviationDenominatorPart<num::PairCalculationFloat<RX, RY>>(sigma_x, sigma_x2, n);
    if (not denominator_x)
    {
        return denominator_x;
    }

    const auto denominator_y = rawDeviationDenominatorPart<num::PairCalculationFloat<RX, RY>>(sigma_y, sigma_y2, n);
    if (not denominator_y)
    {
        return denominator_y;
    }

    const auto denominator = *denominator_x * *denominator_y;
    if (denominator == num::PairCalculationFloat<RX, RY>{0.0})
    {
        return std::unexpected(std::format("denominator is zero?"));
    }

    if constexpr (verboseDebugging)
    {
        println("coefficientCorrelation: count={} sigma_x={} sigma_y={} sigma_xy={} numerator={} denominator_x={} "
                "denominator_y={} denominator={}",
                count,
                sigma_x,
                sigma_y,
                sigma_xy,
                numerator,
                *denominator_x,
                *denominator_y,
                denominator);
    }

    return static_cast<num::PairPublicResultType<RX, RY>>(numerator / denominator);
}

/// @brief Compute the sample covariance of two numeric ranges.
/// @param range_x Input range x.
/// @param range_y Input range y.
/// @details Uses a single fused pass to accumulate n, sigma_x, sigma_y, and sigma_xy
///          simultaneously, avoiding the three separate traversals of the original
///          implementation.
/// @return Covariance on success, or an error if the inputs have mismatched sizes or too few elements.
template <ForwardNumberRange RX, ForwardNumberRange RY>
auto covariance(const RX& range_x, const RY& range_y) -> std::expected<num::PairPublicResultType<RX, RY>, std::string>
{
    // Fused single pass: count n and accumulate sigma_x, sigma_y, sigma_xy simultaneously
    num::PairCalculationFloat<RX, RY> sigma_x{}, sigma_y{}, sigma_xy{};
    std::size_t                       n{};

    auto       itx  = std::ranges::begin(range_x);
    auto       ity  = std::ranges::begin(range_y);
    const auto endx = std::ranges::end(range_x);
    const auto endy = std::ranges::end(range_y);

    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        const auto xi = static_cast<num::PairCalculationFloat<RX, RY>>(*itx);
        const auto yi = static_cast<num::PairCalculationFloat<RX, RY>>(*ity);
        sigma_x += xi;
        sigma_y += yi;
        sigma_xy += xi * yi;
        ++n;
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::format("covariance: ranges have different lengths"));
    }

    if (n < 2)
    {
        return std::unexpected(std::format("not enough data points: count={}", n));
    }

    const auto count       = static_cast<num::PairCalculationFloat<RX, RY>>(n);
    const auto numerator   = sigma_xy - ((sigma_x * sigma_y) / count);
    const auto denominator = static_cast<num::PairCalculationFloat<RX, RY>>(n - 1);
    return static_cast<num::PairPublicResultType<RX, RY>>(numerator / denominator);
}
} // namespace mally::statlib
