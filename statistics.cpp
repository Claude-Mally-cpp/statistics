// @file statistics.cpp
// @brief This file contains functions to compute statistics on a range of numbers.
// @details The functions use high precision floating point types to avoid precision loss.
// @author Claude Mally
#include "statistics.hpp"

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
auto sum(const NumberRange auto& range) -> HighPrecisionFloat
{
    return std::accumulate(
        std::ranges::begin(range), std::ranges::end(range), 0.0L,
        [](HighPrecisionFloat acc, auto val) {
            return acc + static_cast<HighPrecisionFloat>(val);
        });
}

/// @brief compute the average of a range of numbers
/// @param range input range of numbers
/// @return average of the range of numbers
/// @details This function computes the average of a range of numbers. It uses a high precision floating point type to avoid precision loss.
auto average(const NumberRange auto& range) -> HighPrecisionFloat
{
    if (! range.size())
    {
        return 0.0L;
    }

    auto total = sum(range);
    return total / toHPF(range.size());
}

/// @brief compute the sum of squares of a range of numbers
/// @param range input range of numbers
/// @return sum of squares of the range of numbers
auto sumSquared(const NumberRange auto& range) -> HighPrecisionFloat
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
auto sumProduct(const NumberRange auto& rangeX, const NumberRange auto& rangeY) 
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

int main(int argc, const char* argv[])
{
    // lambda function to compute the correlation coefficient between two
    // columns of data. The function takes a title and two columns of data as input.
    // It performs some checks on the size of the columns and computes the correlation coefficient.
    // It outputs the result or an error message to the console.
    auto coefCorrel = [](auto title, const auto& xColumn, const auto& yColumn)
    -> HighPrecisionResult
    {    
        const auto r = coefficientCorrelation(xColumn, yColumn);
        if (!r)
        {
            std::println("{} error: {}", title, r.error());
            return r;
        }
        std::println("{}={}", title, *r);
        return *r;
    };

    const bool rendements_ab_ac_bc = false;
    if constexpr (rendements_ab_ac_bc)
    {
        const auto returnsA = std::array{ 0.07, 0.09, 0.10 };
        const auto returnsB = std::array{ 0.085, 0.07, 0.095 };
        const auto returnsC = std::array{ 0.12, 0.11, 0.10 };
        
        auto rab = coefCorrel("r_ab", returnsA, returnsB);
        auto rac = coefCorrel("r_ac", returnsA, returnsC);
        auto rbc = coefCorrel("r_bc", returnsB, returnsC);
    }

    const bool rendementsTitresXversusRendementsDuMarche = false;
    if constexpr (rendementsTitresXversusRendementsDuMarche)
    {
        const auto rendemetsTitresX = std::array{ -0.10, -0.05, 0.00, 0.08, 0.14, 0.20, 0.25 };
        const auto rendementsMarche = std::array{ -0.20, -0.10, -0.05, 0.00, 0.10, 0.20, 0.30 };
        static_assert(rendemetsTitresX.size() == rendementsMarche.size());
        auto cov_xy = covariance(rendemetsTitresX, rendementsMarche);
        if (!cov_xy)
        {
            std::println("error computing covariance");
        }
        else
        {
            std::println("cov_xy={:.2f}", *cov_xy);
        }
    }

    const bool question29 = true;
    if constexpr (question29)
    {
        const auto profits =
            std::array{
                300, 9'300, 20'900, 31'000, 41'400,
                47'700, 60'800, 79'500, 80'400, 89'000,
                118'300, 119'700, 153'000, 252'800, 333'300,
                412'000, 424'300, 454'000, 829'000, 86'500,
                176'000, 227'400, 471'300, 681'100, 747'000,
                859'800, 939'500, 1'082'000, 1'102'200, 1'495'400
            };
        const auto employers =
            std::array{ 
                7'523, 8'200, 12'068, 9'500, 5'000,
                18'000, 4'708, 13'740, 95'000, 8'200,
                56'000, 31'404, 8'578, 2'900, 9'100,
                10'200, 9'548, 82'300, 28'334, 40'929,
                50'816, 54'100, 28'200, 83'100, 3'418,
                34'400, 42'100, 8'527, 21'300, 20'100
            };
        static_assert(profits.size() == employers.size());

        auto rbc = coefCorrel("r_profit_employers", profits, employers);
    }
}
