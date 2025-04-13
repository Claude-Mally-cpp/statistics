﻿// s tatistics.cpp : Defines the entry point for the application.
//

#include "statistics.hpp"


template<typename T>
concept NumberRange = requires(T t) {
    { t } -> std::ranges::range;
    requires std::integral<std::ranges::range_value_t<T>> || std::floating_point<std::ranges::range_value_t<T>>;
};

auto sum(NumberRange auto range) -> double
{
    auto total = 0.0;
    for (auto sample : range)
    {
        total += sample;
    }
    return total;
}

auto average(NumberRange auto range) -> double
{
    if (! range.size())
    {
        return 0;
    }

    auto total = sum(range);
    return total / range.size();
}

template<std::ranges::input_range Range>
requires std::is_arithmetic_v<std::ranges::range_value_t<Range>>
auto sumSquared(const Range& range) -> double
{
    using Value = std::common_type_t<double, std::ranges::range_value_t<Range>>;
    Value total = 0.0;

    for (const auto& sample : range)
    {
        total += static_cast<Value>(sample) * static_cast<Value>(sample);
    }

    return total;
}

template<std::ranges::input_range RangeX, std::ranges::input_range RangeY>
requires std::is_arithmetic_v<std::ranges::range_value_t<RangeX>> &&
         std::is_arithmetic_v<std::ranges::range_value_t<RangeY>>
auto sumProduct(const RangeX& rangeX, const RangeY& rangeY) -> std::optional<double>
{
    if (std::ranges::distance(rangeX) != std::ranges::distance(rangeY))
    {
        std::println("rangeX.size() = {} != rangeY.size() = {}",
                     std::ranges::distance(rangeX), std::ranges::distance(rangeY));
        return std::nullopt;
    }

    if (std::ranges::empty(rangeX))
    {
        std::println("rangeX is empty!");
        return std::nullopt;
    }

    return std::transform_reduce(
        std::ranges::begin(rangeX), std::ranges::end(rangeX),
        std::ranges::begin(rangeY),
        0.0,
        std::plus{},
        [](auto a, auto b) { return static_cast<double>(a) * static_cast<double>(b); }
    );
}

auto rawDeviationDenominatorPart(auto sum, auto sumSquared, std::size_t n, bool logging = false)
    -> std::optional<double>
{
    const auto radicante = n * sumSquared - sum * sum;
    if (radicante < 0)
    {
        std::println("{} * {} - {}^2={}", n, sumSquared, sum, radicante);
        return {};
    }

    const auto result = std::sqrt(radicante);
    if (logging)
    {
        std::println("radicante={} rawDeviationDenominatorPart={}", radicante, result);
    }

    return result;
}

auto coefficientCorelation(NumberRange auto range_x, NumberRange auto range_y, bool logging=false)
    -> std::optional<double>
{
    auto sigma_x = sum(range_x);
    auto sigma_y = sum(range_y);
    auto sigma_x2 = sumSquared(range_x);
    auto sigma_y2 = sumSquared(range_y);
    auto sigma_xy = sumProduct(range_x, range_y);
    if (!sigma_xy)
    {
        std::println("Unable to compute sigma_xy");
        return {};
    }

    auto n = range_x.size();
    auto numerator = n * *sigma_xy - (sigma_x * sigma_y);

    auto denominator_x = rawDeviationDenominatorPart(sigma_x, sigma_x2, n, logging);
    if (!denominator_x)
    {
        std::println("can't compute rawDeviationDenominatorPart x!");
        std::println("n={} sigma_x={} sigma_x2={}", n, sigma_x, sigma_x2);
        std::println("n={} sigma_y={} sigma_y2={}", n, sigma_y, sigma_y2);
        std::println("sigma_xy={}", *sigma_xy);
        return {};
    }

    auto denominator_y = rawDeviationDenominatorPart(sigma_y, sigma_y2, n, logging);
    if (!denominator_y)
    {
        std::println("can't compute rawDeviationDenominatorPart y!");
        std::println("n={} sigma_x={} sigma_x2={}", n, sigma_x, sigma_x2);
        std::println("n={} sigma_y={} sigma_y2={}", n, sigma_y, sigma_y2);
        std::println("sigma_xy={}", *sigma_xy);
        return {};
    }

    auto denominator = *denominator_x * *denominator_y;
    if (denominator == 0.0)
    {
        std::println("denominator is zero?");
        return {};
    }

    const auto result = numerator / denominator;
    if (logging)
    {
        std::println("n={} sigma_x={} sigma_y={} sigma_xy={}",
            n, sigma_x, sigma_y, *sigma_xy);
        std::println("sigma_x^2={} sigma_y^2={}", sigma_x2, sigma_y2);
        std::println("numerator={} denominator_x={} denominator_y={} denominator={} result={}",
            numerator, *denominator_x, *denominator_y, denominator, result);
    }
    return result;
}

auto covariance(NumberRange auto serie_x, NumberRange auto serie_y)
-> std::optional<double>
{
    if (serie_x.size() != serie_y.size())
    {
        std::println("serie_x.size={} != serie_y.size()={}", serie_x.size(), serie_y.size());
        return {};
    }
    const auto n = serie_x.size();
    if (n < 2)
    {
        std::println("not enough data points: n={}", n);
        return {};
    }

    auto sigma_x = sum(serie_x);
    auto sigma_y = sum(serie_y);
    auto sigma_xy = sumProduct(serie_x, serie_y);
    if (!sigma_xy)
    {
        std::println("error computing sigma_xy");
        return {};
    }
    auto numerator = *sigma_xy - (sigma_x * sigma_y) / (double)n;
    auto denominator = static_cast<double>(n - 1);
    return numerator / denominator;
}

int main(int argc, const char* argv[])
{
    auto coefCorrel = [](auto titre, auto colone_x, auto colone_y)
    -> std::optional<double>
    {
        auto r = coefficientCorelation(colone_x, colone_y, true);
        if (!r)
        {
            std::println("error computing coefficientCorelation {}", titre);
            return {};
        }
        std::println("{}={:.4f}", titre, *r);
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
/*
radicante=141 800 892 690 000 rawDeviationDenominatorPart=11908018.000070373
radicante=573874297961 rawDeviationDenominatorPart=757544.9148142966
n=30 sigma_x=11424900 sigma_y=807293 sigma_xy=325126674200
sigma_x^2=9077641090000 sigma_y^2=40853209527
numerator=530558430300 denominator_x=11908018.000070373 denominator_y=757544.9148142966 denominator=9020858481470.422 result=0.05881462738716168
r_profit_employers=0.0588

n	30
Σx	11 424 900
Σx^2	9 077 641 090 000
Σy	807 293
Σy^2	40 853 209 527
Σxy	325 126 674 200
n Σxy	9 753 800 226 000
Σx * Σy	9 223 241 795 700
n * Σx^2 - (Σx)^2	141 800 892 690 000
n * Σy^2 - (Σy)^2	466 743 181 803 034 000
numérateur	530 558 430 300
dénominateur	8 135 391 805 969 840
r_profit_employés	0,0000652

*/