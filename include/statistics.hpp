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
/// - Result-type policy:
///   - `minMaxValue` preserves the natural input value type.
///   - `sum`, `product`, and `sumSquared` return widened integral types for integral inputs.
///   - `modes` returns all tied repeated modes in `std::expected<std::vector<T>, std::string>`,
///     where `T` is the natural input value type.
///   - `average`, `median`, `quartiles`, `summary`, `variance`,
///     `standardDeviation`, `correlationCoefficient`, and `covariance`
///     follow the statistical public-result policy.
///   - Internal calculation may widen independently from the public return type.
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
#include <expected>
#include <format>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

namespace mally::statlib
{

/// @brief Enable verbose debugging output to stderr (for development).
/// @note Disabled by default; enable manually when needed.
inline constexpr bool verboseDebugging = false;

/// @brief Variance / standard-deviation denominator convention.
enum class VarianceKind
{
    sample,
    population,
};

using num::ForwardNumberRange;
using num::NumberRange;

namespace detail
{
/// @brief One-pass accumulation state for univariate dispersion calculations.
/// @tparam CalcT Internal widened calculation type.
template <class CalcT> struct DispersionAccumulation
{
    CalcT       sum{};        ///< Sum of values.
    CalcT       sumSquares{}; ///< Sum of squared values.
    std::size_t count{};      ///< Number of observed values.
};

/// @brief One-pass accumulation state for paired dispersion and covariance calculations.
/// @tparam CalcT Internal widened calculation type.
template <class CalcT> struct BivariateAccumulation
{
    CalcT       sumX{};        ///< Sum of x values.
    CalcT       sumY{};        ///< Sum of y values.
    CalcT       sumSquaresX{}; ///< Sum of squared x values.
    CalcT       sumSquaresY{}; ///< Sum of squared y values.
    CalcT       sumProducts{}; ///< Sum of pairwise products x*y.
    std::size_t count{};       ///< Number of paired observations.
};

/// @brief Accumulate count, sum, and sum of squares for a numeric range.
/// @param range Input range.
/// @return Widened accumulation state for variance-style formulas.
template <ForwardNumberRange R> constexpr auto accumulateDispersion(const R& range) -> DispersionAccumulation<num::RangeCalculationFloat<R>>
{
    DispersionAccumulation<num::RangeCalculationFloat<R>> accumulation{};
    for (auto&& value : range)
    {
        const auto widenedValue = static_cast<num::RangeCalculationFloat<R>>(value);
        accumulation.sum += widenedValue;
        accumulation.sumSquares += widenedValue * widenedValue;
        ++accumulation.count;
    }
    return accumulation;
}

/// @brief Accumulate paired sums, sums of squares, and cross-products for two ranges.
/// @param range_x First input range.
/// @param range_y Second input range.
/// @return Widened paired accumulation state, or an error on length mismatch.
template <ForwardNumberRange RX, ForwardNumberRange RY>
auto accumulateBivariate(const RX& range_x, const RY& range_y)
    -> std::expected<BivariateAccumulation<num::PairCalculationFloat<RX, RY>>, std::string>
{
    BivariateAccumulation<num::PairCalculationFloat<RX, RY>> accumulation{};

    auto       itx  = std::ranges::begin(range_x);
    auto       ity  = std::ranges::begin(range_y);
    const auto endx = std::ranges::end(range_x);
    const auto endy = std::ranges::end(range_y);

    for (; itx != endx && ity != endy; ++itx, ++ity)
    {
        const auto xValue = static_cast<num::PairCalculationFloat<RX, RY>>(*itx);
        const auto yValue = static_cast<num::PairCalculationFloat<RX, RY>>(*ity);
        accumulation.sumX += xValue;
        accumulation.sumY += yValue;
        accumulation.sumSquaresX += xValue * xValue;
        accumulation.sumSquaresY += yValue * yValue;
        accumulation.sumProducts += xValue * yValue;
        ++accumulation.count;
    }

    if (itx != endx || ity != endy)
    {
        return std::unexpected(std::string{"ranges have different lengths"});
    }

    return accumulation;
}

/// @brief Compute the centered sum-of-squares term `sum(x^2) - sum(x)^2 / n`.
/// @param sum Sum of values.
/// @param sumSquares Sum of squared values.
/// @param n Number of values.
/// @return Centered sum-of-squares term, or an error for empty input / invalid negative result.
template <class CalcT> constexpr auto centeredSquareSum(CalcT sum, CalcT sumSquares, std::size_t n) -> std::expected<CalcT, std::string>
{
    if (n == 0)
    {
        return std::unexpected(std::string{"empty range"});
    }

    const auto count = static_cast<CalcT>(n);
    const auto value = sumSquares - ((sum * sum) / count);
    if (value < 0)
    {
        return std::unexpected(std::format("sumSquares={} - (sum={}^2 / n={})={}", sumSquares, sum, n, value));
    }
    return value;
}

/// @brief Compute the centered cross-product term `sum(xy) - sum(x)sum(y) / n`.
/// @param sum_x Sum of x values.
/// @param sum_y Sum of y values.
/// @param sum_xy Sum of pairwise products.
/// @param n Number of paired values.
/// @return Centered cross-product term, or an error for empty input.
template <class CalcT>
constexpr auto centeredCrossSum(CalcT sum_x, CalcT sum_y, CalcT sum_xy, std::size_t n) -> std::expected<CalcT, std::string>
{
    if (n == 0)
    {
        return std::unexpected(std::string{"empty range"});
    }

    const auto count = static_cast<CalcT>(n);
    return sum_xy - ((sum_x * sum_y) / count);
}

/// @brief Select the variance divisor for sample or population convention.
/// @param n Number of values.
/// @param kind Variance convention selector.
/// @return Divisor value, or an error for empty / too-small sample input.
template <class CalcT> constexpr auto varianceDivisor(std::size_t n, VarianceKind kind) -> std::expected<CalcT, std::string>
{
    if (n == 0)
    {
        return std::unexpected(std::string{"variance: empty range"});
    }

    if (kind == VarianceKind::sample)
    {
        if (n < 2)
        {
            return std::unexpected(std::format("variance: sample variance requires at least 2 values, count={}", n));
        }
        return static_cast<CalcT>(n - 1);
    }

    return static_cast<CalcT>(n);
}
} // namespace detail

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
    const auto quartileSummary = quartilesFromSortedSpan<num::RangeValueType<R>>(sortedValues);
    out.q1                     = quartileSummary.q1;
    out.median                 = quartileSummary.median;
    out.q3                     = quartileSummary.q3;

    return out;
}

/// @brief Compute the product of a range of numbers.
/// @param range Input range of numbers.
/// @details Uses widened internal accumulation and returns a widened integral result for integral inputs.
/// @return Product of all values in `range`.
template <NumberRange R> constexpr auto product(const R& range) -> num::RangeNaturalArithmeticResultType<R>
{
    using AccumulatorType = std::
        conditional_t<std::is_integral_v<num::RangeValueType<R>>, num::RangeNaturalArithmeticResultType<R>, num::RangeCalculationFloat<R>>;
    const auto value = std::accumulate(std::ranges::begin(range),
                                       std::ranges::end(range),
                                       AccumulatorType{1},
                                       [](AccumulatorType acc, auto val) -> auto { return acc * static_cast<AccumulatorType>(val); });
    return static_cast<num::RangeNaturalArithmeticResultType<R>>(value);
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

    const auto value = std::pow(totalProduct, num::RangeCalculationFloat<R>{1.0} / static_cast<num::RangeCalculationFloat<R>>(count));
    return static_cast<num::RangePublicResultType<R>>(value);
}

/// @brief Compute the sum of squares of a range of numbers.
/// @param range Input range of numbers.
/// @details Uses widened internal accumulation and returns a widened integral result for integral inputs.
/// @return Sum of squared values in `range`.
template <NumberRange R> constexpr auto sumSquared(const R& range) -> num::RangeNaturalArithmeticResultType<R>
{
    using AccumulatorType = std::
        conditional_t<std::is_integral_v<num::RangeValueType<R>>, num::RangeNaturalArithmeticResultType<R>, num::RangeCalculationFloat<R>>;
    const auto value = std::accumulate(std::ranges::begin(range),
                                       std::ranges::end(range),
                                       AccumulatorType{},
                                       [](AccumulatorType acc, auto val) -> auto
                                       {
                                           const auto widenedValue = static_cast<AccumulatorType>(val);
                                           const auto valueSquared = widenedValue * widenedValue;
                                           return acc + valueSquared;
                                       });
    return static_cast<num::RangeNaturalArithmeticResultType<R>>(value);
}

/// @brief Compute the variance of a numeric range.
/// @param range Input range of numbers.
/// @param kind Sample or population convention. Defaults to sample variance.
/// @return Variance on success, or an error for empty input / too-few sample values.
template <ForwardNumberRange R>
auto variance(const R& range, VarianceKind kind = VarianceKind::sample) -> std::expected<num::RangePublicResultType<R>, std::string>
{
    const auto accumulation = detail::accumulateDispersion(range);
    const auto numerator    = detail::centeredSquareSum(accumulation.sum, accumulation.sumSquares, accumulation.count);
    if (not numerator)
    {
        return std::unexpected(std::format("variance: {}", numerator.error()));
    }

    const auto divisor = detail::varianceDivisor<num::RangeCalculationFloat<R>>(accumulation.count, kind);
    if (not divisor)
    {
        return std::unexpected(divisor.error());
    }

    return static_cast<num::RangePublicResultType<R>>(*numerator / *divisor);
}

/// @brief Compute the standard deviation of a numeric range.
/// @param range Input range of numbers.
/// @param kind Sample or population convention. Defaults to sample standard deviation.
/// @return Standard deviation on success, or an error for empty input / too-few sample values.
template <ForwardNumberRange R>
auto standardDeviation(const R& range, VarianceKind kind = VarianceKind::sample)
    -> std::expected<num::RangePublicResultType<R>, std::string>
{
    const auto varianceValue = variance(range, kind);
    if (not varianceValue)
    {
        return std::unexpected(std::format("standardDeviation: {}", varianceValue.error()));
    }

    return static_cast<num::RangePublicResultType<R>>(std::sqrt(static_cast<num::RangeCalculationFloat<R>>(*varianceValue)));
}

/// @brief Compute the repeated modes of a numeric range.
/// @param range Input range of numbers.
/// @details Returns all repeated values tied for highest frequency in sorted order.
///          Returns `unexpected` for empty input or when no value is repeated.
/// @return `std::expected<std::vector<T>, std::string>`, where `T` is the natural input value type.
template <NumberRange R> auto modes(const R& range) -> std::expected<std::vector<num::RangeValueType<R>>, std::string>
{
    std::vector<num::RangeValueType<R>> sortedValues;
    if constexpr (std::ranges::sized_range<R>)
    {
        sortedValues.reserve(static_cast<std::size_t>(std::ranges::size(range)));
    }
    for (auto&& value : range)
    {
        sortedValues.push_back(static_cast<num::RangeValueType<R>>(value));
    }

    if (sortedValues.empty())
    {
        return std::unexpected(std::string{"modes: empty range"});
    }

    std::ranges::sort(sortedValues);

    std::vector<num::RangeValueType<R>> modeValues;
    std::size_t                         bestCount = 0;

    for (auto it = sortedValues.begin(); it != sortedValues.end();)
    {
        auto       runEnd   = std::ranges::upper_bound(sortedValues, *it);
        const auto runCount = static_cast<std::size_t>(std::ranges::distance(it, runEnd));

        if (runCount > bestCount)
        {
            bestCount = runCount;
            modeValues.assign(1, *it);
        }
        else if (runCount == bestCount)
        {
            modeValues.push_back(*it);
        }

        it = runEnd;
    }

    if (bestCount <= 1)
    {
        return std::unexpected(std::string{"modes: no repeated value"});
    }

    return modeValues;
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
    const auto centeredSquareSumValue = detail::centeredSquareSum(static_cast<CalcT>(sum), static_cast<CalcT>(sumSquared), n);
    if (not centeredSquareSumValue)
    {
        return std::unexpected(centeredSquareSumValue.error());
    }

    if constexpr (verboseDebugging)
    {
        println("rawDeviationDenominatorPart: n={} sum={} sumSquared={} centeredSquareSum={}", n, sum, sumSquared, *centeredSquareSumValue);
    }

    return std::sqrt(static_cast<CalcT>(n) * *centeredSquareSumValue);
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
    const auto accumulation = detail::accumulateBivariate(range_x, range_y);
    if (not accumulation)
    {
        return std::unexpected(std::format("correlationCoefficient: {}", accumulation.error()));
    }

    if (accumulation->count < 2)
    {
        return std::unexpected(std::format("not enough data points: n={}", accumulation->count));
    }

    const auto count = static_cast<num::PairCalculationFloat<RX, RY>>(accumulation->count);
    const auto centeredCrossTerm =
        detail::centeredCrossSum(accumulation->sumX, accumulation->sumY, accumulation->sumProducts, accumulation->count);
    if (not centeredCrossTerm)
    {
        return std::unexpected(std::format("correlationCoefficient: {}", centeredCrossTerm.error()));
    }

    const auto numerator = count * *centeredCrossTerm;
    if constexpr (verboseDebugging)
    {
        println("count={} sigma_x={} sigma_y={} sigma_xy={} numerator={}",
                count,
                accumulation->sumX,
                accumulation->sumY,
                accumulation->sumProducts,
                numerator);
    }

    const auto denominator_x =
        rawDeviationDenominatorPart<num::PairCalculationFloat<RX, RY>>(accumulation->sumX, accumulation->sumSquaresX, accumulation->count);
    if (not denominator_x)
    {
        return denominator_x;
    }

    const auto denominator_y =
        rawDeviationDenominatorPart<num::PairCalculationFloat<RX, RY>>(accumulation->sumY, accumulation->sumSquaresY, accumulation->count);
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
                accumulation->sumX,
                accumulation->sumY,
                accumulation->sumProducts,
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
    const auto accumulation = detail::accumulateBivariate(range_x, range_y);
    if (not accumulation)
    {
        return std::unexpected(std::format("covariance: {}", accumulation.error()));
    }

    if (accumulation->count < 2)
    {
        return std::unexpected(std::format("not enough data points: count={}", accumulation->count));
    }

    const auto numerator = detail::centeredCrossSum(accumulation->sumX, accumulation->sumY, accumulation->sumProducts, accumulation->count);
    if (not numerator)
    {
        return std::unexpected(std::format("covariance: {}", numerator.error()));
    }

    const auto denominator = static_cast<num::PairCalculationFloat<RX, RY>>(accumulation->count - 1);
    return static_cast<num::PairPublicResultType<RX, RY>>(*numerator / denominator);
}
} // namespace mally::statlib
