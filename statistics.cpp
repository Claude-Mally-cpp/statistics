// @file statistics.cpp
#include "statistics.hpp"
#include "print_compat.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>

namespace mally::statlib
{

int test()
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
            mally::statlib::println("{} error: {}", title, r.error());
            return r;
        }
        mally::statlib::println("{}={}", title, *r);
        return *r;
    };

    const bool rendements_ab_ac_bc = true;
    if constexpr (rendements_ab_ac_bc)
    {
        const auto returnsA = std::array{ 0.07, 0.09, 0.10 };
        const auto returnsB = std::array{ 0.085, 0.07, 0.095 };
        const auto returnsC = std::array{ 0.12, 0.11, 0.10 };
        
        auto rab = coefCorrel("r_ab", returnsA, returnsB);
        auto rac = coefCorrel("r_ac", returnsA, returnsC);
        auto rbc = coefCorrel("r_bc", returnsB, returnsC);
    }

    const bool rendementsTitresXversusRendementsDuMarche = true;
    if constexpr (rendementsTitresXversusRendementsDuMarche)
    {
        constexpr auto rendemetsTitresX = std::array{ -0.10, -0.05, 0.00, 0.08, 0.14, 0.20, 0.25 };
        constexpr auto rendementsMarche = std::array{ -0.20, -0.10, -0.05, 0.00, 0.10, 0.20, 0.30 };
        static_assert(rendemetsTitresX.size() == rendementsMarche.size());
        auto cov_xy = covariance(rendemetsTitresX, rendementsMarche);
        if (!cov_xy)
        {
            mally::statlib::println("error computing covariance");
            return -1;
        }
        else
        {
            mally::statlib::println("cov_xy={:.2f}", *cov_xy);
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

    const auto testProduct = true;
    if constexpr (testProduct)
    {
        constexpr auto productTest = std::array{ 1, 2, 3, 4, 5 };
        static_assert(product(productTest) == 120);

        constexpr auto insectCount =
            std::array{10, 1, 1000, 1, 10};
        constexpr auto result = product(insectCount);
        static_assert(result == 100000);
        // Use fmt::println directly for container printing
        fmt::println("product({})={:0.2Lf}", insectCount, product(insectCount));
    }

    // Note: The function returns 0 to indicate successful execution.
    return 0;
}
}

int main(int argc, const char* argv[])
{
    return mally::statlib::test();;
}
