/// @file statistics.hpp : minimalist statistics library
/// @brief This file contains functions to compute statistics on range(s) of numbers.
/// @details The functions use high precision floating point types to avoid precision loss.
/// @author Claude Mally
/// @date 2025-04-11
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <stdint.h>
#include <type_traits> 
#include <vector>

// @brief This file contains functions to compute statistics on a range of numbers.
// @details The functions use high precision floating point types to avoid precision loss.
namespace mally::statlib
{
    /// @brief Concept for a range of numbers.
    template<typename T>
    concept NumberRange = requires(T t) {
        std::ranges::range<T> &&
        std::is_arithmetic_v<std::ranges::range_value_t<T>>;
    };

    /// @brief Type used for high precision floating point calculations.
    /// @details This type is used to avoid precision loss when calculating
    using HighPrecisionFloat = long double;

    /// @brief Type used for high precision floating point calculations with expected result.
    /// @details The unexpected result is a string error message.
    using HighPrecisionResult = std::expected<HighPrecisionFloat, std::string>;

    /// @brief Convert a value to a high precision floating point type.
    /// @param value value to convert
    /// @return HighPrecisionFloat from the value
    /// @details This function converts a value to a high precision floating point type.
    constexpr auto toHPF(auto value) -> HighPrecisionFloat
    {
        return static_cast<HighPrecisionFloat>(value);
    }

    /// @brief Verbose debugging flag.
    /// @details This flag is used to enable verbose debugging output.
    const auto verboseDebugging = false;


    /// @brief compute the sum of a range of numbers
    /// @param range input range of numbers
    /// @details This function computes the sum of a range of numbers. It uses a high precision floating point type to avoid precision loss.
    /// @return sum of the range of numbers
    constexpr auto sum(const NumberRange auto& range) -> HighPrecisionFloat
    {
        return std::accumulate(
            std::ranges::begin(range), std::ranges::end(range), 0.0L,
            [](HighPrecisionFloat acc, auto val) {
                return acc + toHPF(val);
            });
    }

    /// @brief compute the average of a range of numbers
    /// @param range input range of numbers
    /// @return average of the range of numbers
    /// @details This function computes the average of a range of numbers.
    /// It uses a high precision floating point type to avoid precision loss.
    constexpr auto average(const NumberRange auto& range) -> HighPrecisionFloat
    {
        if (! range.size())
        {
            return 0.0L;
        }

        auto total = sum(range);
        return total / toHPF(range.size());
    }

    /// @brief compute the product of a range of numbers
    /// @param range input range of numbers
    /// @details This function computes the producy of a range of numbers.
    /// It uses a high precision floating point type to avoid precision loss.
    /// @return ptoduct of the range of numbers
    constexpr auto product(const NumberRange auto& range) -> HighPrecisionFloat
    {
        return std::accumulate(
            std::ranges::begin(range), std::ranges::end(range), 1.0L,
            [](HighPrecisionFloat acc, auto val) {
                return acc * toHPF(val);
            });
    }

    /// @brief compute the geometric mean of a range of numbers
    /// @param range input range of numbers
    /// @return geometric mean of the range of numbers
    /// @details This function computes the average of a range of numbers. 
    /// It usses a high precision floating point type to avoid precision loss.
    constexpr auto geometricMean(const NumberRange auto& range) -> HighPrecisionFloat
    {
        if (! range.size())
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
        return std::accumulate(
            std::ranges::begin(range), std::ranges::end(range), 0.0L,
            [](HighPrecisionFloat acc, auto val) {
                const auto valueSquared = toHPF(val) * toHPF(val);
                return acc + valueSquared;
            });
    }

    /// @brief compute the sum of products of two ranges of numbers
    /// @param rangeX input range x of numbers
    /// @param rangeY input range y of numbers
    /// @return sum of products of the two ranges of numbers
    constexpr auto sumProduct(const NumberRange auto& rangeX, const NumberRange auto& rangeY) 
        -> HighPrecisionResult
    {
        if (std::ranges::distance(rangeX) != std::ranges::distance(rangeY))
        {
            return std::unexpected(
                std::format(
                    "rangeX.size() = {} != rangeY.size() = {}",
                    std::ranges::distance(rangeX),
                    std::ranges::distance(rangeY)
                )
            );
        }

        if (std::ranges::empty(rangeX))
        {
            return std::unexpected("rangeX is empty!");
        }

        auto total = std::transform_reduce(
            std::ranges::begin(rangeX), std::ranges::end(rangeX),
            std::ranges::begin(rangeY), 0.0L,
            std::plus<>{},
            [](auto x, auto y) -> HighPrecisionFloat{
                return toHPF(x) * toHPF(y);
            }
        );

        if (total < 0)
        {
            return std::unexpected(
                std::format("total {} is negative!", total)
            );
        }    

        return total;
    }

    /// @brief Reusable part of the denominator of the correlation coefficient formula
    /// @details This function computes either the x or the y denominator portion of
    /// the correlation coefficient formula:
    /// sqrt(n * sum(x^2) - (sum(x))^2) * sqrt(n * sum(y^2) - (sum(y))^2)
    /// @param sum Sum of the elements in the range
    /// @param sumSquared Sum of squares of the elements in the range
    /// @param n Number of elements in the range
    /// @return HighPrecisionResult
    auto rawDeviationDenominatorPart(auto sum, auto sumSquared, std::size_t n)
        -> HighPrecisionResult
    {
        const auto n_ld = toHPF(n);
        const auto sum_ld = toHPF(sum);
        const auto sumSquared_ld = toHPF(sumSquared);

        const auto radicand = n_ld * sumSquared_ld - sum_ld * sum_ld;
        if (radicand < 0)
        {
            return std::unexpected(
                std::format("{} * {} - {}^2={}", n, sumSquared, sum, radicand)
            );
        }

        if constexpr (verboseDebugging)
        {
            std::println("rawDeviationDenominatorPart: n={} sum={} sumSquared={} radicand={}", n, sum, sumSquared, radicand);
        }

        return std::sqrt(radicand);
    }

    auto coefficientCorrelation(
        const NumberRange auto& range_x,
        const NumberRange auto& range_y
    ) -> HighPrecisionResult
    {
        const auto sizeX = std::ranges::distance(range_x);
        const auto sizeY = std::ranges::distance(range_y);

        if (sizeX != sizeY)
        {
            return std::unexpected(
                std::format("sizeX={} != sizeY()={}", sizeX, sizeY)
            );
        }

        if (sizeX < 2)
        {
            return std::unexpected(
                std::format("not enough data points: n={}", sizeX)
            );
        }
        
        const auto sigma_x = sum(range_x);
        const auto sigma_y = sum(range_y);
        const auto sigma_x2 = sumSquared(range_x);
        const auto sigma_y2 = sumSquared(range_y);
        const auto sigma_xy = sumProduct(range_x, range_y);
        if (!sigma_xy)
        {
            return sigma_xy;
        }

        const auto n = toHPF(range_x.size());
        const auto numerator = toHPF(n) * *sigma_xy - sigma_x * sigma_y;
        if constexpr (verboseDebugging)
        {
            std::println("n={} sigma_x={} sigma_y={} sigma_xy={} numerator={}", n, sigma_x, sigma_y, *sigma_xy, numerator);
        }

        const auto denominator_x = rawDeviationDenominatorPart(sigma_x, sigma_x2, static_cast<std::size_t>(n));
        if (!denominator_x)
        {
            return denominator_x;
        }

        const auto denominator_y = rawDeviationDenominatorPart(sigma_y, sigma_y2, static_cast<std::size_t>(n));
        if (!denominator_y)
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
            std::println("coefficientCorrelation: n={} sigma_x={} sigma_y={} sigma_xy={} numerator={} denominator_x={} denominator_y={} denominator={}", n, sigma_x, sigma_y, *sigma_xy, numerator, *denominator_x, *denominator_y, denominator);
        }

        return numerator / denominator;
    }

    /// @brief compute the covariance of two ranges of numbers
    /// @param range_x input range x of numbers
    /// @param range_y input range y of numbers
    /// @return covariance of the two ranges of numbers
    auto covariance(const NumberRange auto& range_x, const NumberRange auto& range_y)
    -> HighPrecisionResult
    {
        const auto sizeX = std::ranges::distance(range_x);
        const auto sizeY = std::ranges::distance(range_y);

        if (sizeX != sizeY)
        {
            return std::unexpected(
                std::format("sizeX={} != sizeY={}", sizeX, sizeY)
            );
        }

        const auto n = sizeX;
        if (n < 2)
        {
            return std::unexpected(
                std::format("not enough data points: n={}", n)
            );
        }

        const auto sigma_x = sum(range_x);
        const auto sigma_y = sum(range_y);
        const auto sigma_xy = sumProduct(range_x, range_y);
        if (!sigma_xy)
        {
            return sigma_xy;
        }

        const auto numerator = *sigma_xy - (sigma_x * sigma_y) / (HighPrecisionFloat)n;
        const auto denominator = toHPF(n - 1);
        return numerator / denominator;
    }
} // namespace statistics