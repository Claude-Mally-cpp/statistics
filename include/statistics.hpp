/// @file statistics.hpp : minimalist statistics library
/// @brief This file contains functions to compute statistics on range(s) of numbers.
/// @details The functions use high precision floating point types to avoid precision loss.
/// @author Claude Mally
/// @date 2025-04-11
///
/// Notes and conventions
/// - Header location: this file is intended to be installed/used from the project's
///   `include/` directory. When building, add the repository root `include` folder to
///   your compiler's include path and then `#include "statistics.hpp"`.
/// - Quartile convention: this library computes quartiles using the Tukey hinge style.
///   By default the implementation uses the exclusive-median variant (the median element
///   is excluded from both lower and upper halves when computing Q1 and Q3). For a
///   very small sample (size == 3) a small-sample special-case is applied and the
///   median is included in both halves to match common textbook examples. See
///   `quartiles.hpp` and the unit tests under `test/test_statistics.cpp` for details.
/// - Summary output: `summary(range)` returns min, Q1, median, mean, Q3 and max; mean
///   is computed with `average(range)` which returns 0 on empty ranges.
///
/// Example usage:
///   #include "statistics.hpp"
///   using namespace mally::statlib;
///   auto s = summary(myVector);
// ... your existing includes ...
#pragma once

#include "HighPrecisionFloat.hpp"
#include "numeric.hpp"
#include "quartiles.hpp"
#include "summaryStats.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <numeric>
#include <ranges>
#include <type_traits>

namespace mally::statlib
{

/// @brief Enable verbose debugging output to stderr (for development).
/// @note Disabled by default; enable manually when needed.
inline constexpr bool verboseDebugging = false;

using num::NumberRange;

/// @brief Summary for an std::array without allocations.
/// @tparam T arithmetic or HighPrecisionFloat.
/// @note Converts to HPF, sorts a local array, reuses quartilesSorted().
template <class T, std::size_t N>
    requires(std::is_arithmetic_v<std::remove_cvref_t<T>> || std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr auto summary(const std::array<T, N>& data) -> SummaryStats
{
    SummaryStats out{};
    out.count = N;

    if constexpr (N == 0)
    {
        return out;
    }
    else
    {
        // Materialize as HPF and sort
        std::array<HighPrecisionFloat, N> hpArray{};
        for (std::size_t i = 0; i < N; ++i)
        {
            hpArray[i] = toHPF(data[i]);
        }
        std::ranges::sort(hpArray);

        // Min/Max from sorted array
        out.min = hpArray.front();
        out.max = hpArray.back();

        // Quartiles from sorted array
        const auto qSorted = quartilesSorted(hpArray);
        out.q1        = qSorted.q1;
        out.median    = qSorted.median;
        out.q3        = qSorted.q3;

        // Mean (use numeric helper on the original array to avoid re-summing hp)
        out.mean = num::average(data);
        return out;
    }
}

/// @brief Summary for a generic numeric range (vectors, spans, views…).
/// @details Uses numeric helpers and the range-generic quartiles adapter.
/// @note Requires input_range; min/max needs forward iteration (met by std::vector).
template <class R>
    requires num::NumberRange<R>
constexpr auto summary(const R& range) -> SummaryStats
{
    SummaryStats out{};

    if (std::ranges::empty(range))
    {
        return out;
    }

    out.count = static_cast<std::size_t>(std::ranges::distance(range));

    // Min/Max & Mean via helpers (HPF-safe)
    auto [minVal, maxVal] = num::minMaxValue(range);
    out.min               = minVal;
    out.max               = maxVal;
    out.mean              = num::average(range);

    // Quartiles via adapter (materializes to vector<HPF> internally)
    const auto qSorted = quartiles(range);
    out.q1        = qSorted.q1;
    out.median    = qSorted.median;
    out.q3        = qSorted.q3;

    return out;
}

/// @brief compute the product of a range of numbers
/// @param range input range of numbers
/// @details This function computes the product of a range of numbers.
/// It uses a high precision floating point type to avoid precision loss.
/// @return product of the range of numbers
constexpr auto product(const NumberRange auto& range) -> HighPrecisionFloat
{
    return std::accumulate(std::ranges::begin(range), std::ranges::end(range), 1.0L,
                           [](HighPrecisionFloat acc, auto val) -> auto { return acc * toHPF(val); });
}

/// @brief compute the geometric mean of a range of numbers
/// @param range input range of numbers
/// @return geometric mean of the range of numbers
/// @details This function computes the average of a range of numbers.
/// It usses a high precision floating point type to avoid precision loss.
constexpr auto geometricMean(const NumberRange auto& range) -> HighPrecisionFloat
{
    if (not range.size())
    {
        return 0.0L;
    }

    auto totalProduct = product(range);
    return std::powl(totalProduct, 1.0 / toHPF(range.size()));
}

/// @brief compute the sum of squares of a range of numbers
/// @param range input range of numbers
/// @return sum of squares of the range of numbers
constexpr auto sumSquared(const NumberRange auto& range) -> HighPrecisionFloat
{
    return std::accumulate(std::ranges::begin(range), std::ranges::end(range), 0.0L,
                           [](HighPrecisionFloat acc, auto val) -> auto
                           {
                               const auto valueSquared = toHPF(val) * toHPF(val);
                               return acc + valueSquared;
                           });
}

/// @brief Reusable part of the denominator of the correlation coefficient formula
/// @details This function computes either the x or the y denominator portion of
/// the correlation coefficient formula:
/// sqrt(n * sum(x^2) - (sum(x))^2) * sqrt(n * sum(y^2) - (sum(y))^2)
/// @param sum Sum of the elements in the range
/// @param sumSquared Sum of squares of the elements in the range
/// @param n Number of elements in the range
/// @return HighPrecisionResult
auto rawDeviationDenominatorPart(auto sum, auto sumSquared, std::size_t n) -> HighPrecisionResult
{
    const auto n_ld          = toHPF(n);
    const auto sum_ld        = toHPF(sum);
    const auto sumSquared_ld = toHPF(sumSquared);

    const auto radicand = (n_ld * sumSquared_ld) - (sum_ld * sum_ld);
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

auto correlationCoefficient(const NumberRange auto& range_x, const NumberRange auto& range_y) -> HighPrecisionResult
{
    const auto sizeX = std::ranges::distance(range_x);
    const auto sizeY = std::ranges::distance(range_y);

    if (sizeX != sizeY)
    {
        return std::unexpected(std::format("sizeX={} != sizeY()={}", sizeX, sizeY));
    }

    if (sizeX < 2)
    {
        return std::unexpected(std::format("not enough data points: n={}", sizeX));
    }

    const auto sigma_x  = num::sum(range_x);
    const auto sigma_y  = num::sum(range_y);
    const auto sigma_x2 = sumSquared(range_x);
    const auto sigma_y2 = sumSquared(range_y);
    const auto sigma_xy = num::sumProduct(range_x, range_y);
    if (not sigma_xy)
    {
        return sigma_xy;
    }

    const auto count         = toHPF(range_x.size());
    const auto numerator = (count * *sigma_xy) - (sigma_x * sigma_y);
    if constexpr (verboseDebugging)
    {
        println("count={} sigma_x={} sigma_y={} sigma_xy={} numerator={}", count, sigma_x, sigma_y, *sigma_xy, numerator);
    }

    const auto denominator_x = rawDeviationDenominatorPart(sigma_x, sigma_x2, static_cast<std::size_t>(count));
    if (not denominator_x)
    {
        return denominator_x;
    }

    const auto denominator_y = rawDeviationDenominatorPart(sigma_y, sigma_y2, static_cast<std::size_t>(count));
    if (not denominator_y)
    {
        return denominator_y;
    }

    const auto denominator = *denominator_x * *denominator_y;
    if (denominator == 0.0L)
    {
        return std::unexpected(std::format("denominator is zero?"));
    }

    if constexpr (verboseDebugging)
    {
        println("coefficientCorrelation: count={} sigma_x={} sigma_y={} sigma_xy={} numerator={} denominator_x={} "
                "denominator_y={} denominator={}",
                count, sigma_x, sigma_y, *sigma_xy, numerator, *denominator_x, *denominator_y, denominator);
    }

    return numerator / denominator;
}

/// @brief compute the covariance of two ranges of numbers
/// @param range_x input range x of numbers
/// @param range_y input range y of numbers
/// @return covariance of the two ranges of numbers
auto covariance(const NumberRange auto& range_x, const NumberRange auto& range_y) -> HighPrecisionResult
{
    const auto sizeX = std::ranges::distance(range_x);
    const auto sizeY = std::ranges::distance(range_y);

    if (sizeX != sizeY)
    {
        return std::unexpected(std::format("sizeX={} != sizeY={}", sizeX, sizeY));
    }

    const auto count = sizeX;
    if (count < 2)
    {
        return std::unexpected(std::format("not enough data points: count={}", count));
    }

    const auto sigma_x  = num::sum(range_x);
    const auto sigma_y  = num::sum(range_y);
    const auto sigma_xy = num::sumProduct(range_x, range_y);
    if (not sigma_xy)
    {
        return sigma_xy;
    }

    const auto numerator   = *sigma_xy - ((sigma_x * sigma_y) / toHPF(count));
    const auto denominator = toHPF(count - 1);
    return numerator / denominator;
}
} // namespace mally::statlib
