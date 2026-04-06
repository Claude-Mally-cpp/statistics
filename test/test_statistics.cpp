// test_statistics.cpp
#include "HighPrecisionFloat.hpp"
#include "quartiles.hpp"
#include "statistics.hpp"
#include <array>
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using namespace mally::statlib;

// Test correlation between returnsA and returnsB
TEST(StatisticsTest, Correlation_AB)
{
    const std::array<double, 3> returnsA = {0.07, 0.09, 0.10};
    const std::array<double, 3> returnsB = {0.085, 0.07, 0.095};
    auto                        result   = correlationCoefficient(returnsA, returnsB);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    // Use fixed expected value, update if implementation changes
    auto       expected  = 0.21677749238102959L;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance)
        << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
    // Alternative: test for range, or parameterize with more datasets
}

// Test correlation between returnsA and returnsC
TEST(StatisticsTest, Correlation_AC)
{
    const auto returnsA = std::array<double, 3>{0.07, 0.09, 0.10};
    const auto returnsC = std::array<double, 3>{0.12, 0.11, 0.10};
    auto       result   = correlationCoefficient(returnsA, returnsC);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    auto       expected  = -0.9819805060619121L;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance)
        << "Expected " << -0.981980 << ", got " << *result << ". Check calculation or expected value.";
}

// Test correlation between returnsB and returnsC
TEST(StatisticsTest, Correlation_BC)
{
    const auto returnsB = std::array{0.085, 0.07, 0.095};
    const auto returnsC = std::array{0.12, 0.11, 0.10};
    auto       result   = correlationCoefficient(returnsB, returnsC);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    auto       expected  = -0.39735970711947155L;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance)
        << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
}

// Test covariance between rendemetsTitresX and rendementsMarche
TEST(StatisticsTest, Covariance_TitresX_Marche)
{
    constexpr auto returnsX      = std::array{-0.10, -0.05, 0.00, 0.08, 0.14, 0.20, 0.25};
    constexpr auto marketReturns = std::array{-0.20, -0.10, -0.05, 0.00, 0.10, 0.20, 0.30};
    auto           result        = covariance(returnsX, marketReturns);
    ASSERT_TRUE(result.has_value());
    auto       expected  = 0.022571428571428576L;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance)
        << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
    // Alternative: test for symmetry, or with randomized data
}

// Test correlation between profits and employers
TEST(StatisticsTest, Correlation_Profits_Employers)
{
    const auto profits   = std::array{300,    9300,   20900,  31000,  41400,  47700,  60800,  79500,   80400,   89000,
                                      118300, 119700, 153000, 252800, 333300, 412000, 424300, 454000,  829000,  86500,
                                      176000, 227400, 471300, 681100, 747000, 859800, 939500, 1082000, 1102200, 1495400};
    const auto employers = std::array{7523,  8200, 12068, 9500,  5000,  18000, 4708,  13740, 95000, 8200, 56000, 31404, 8578, 2900,  9100,
                                      10200, 9548, 82300, 28334, 40929, 50816, 54100, 28200, 83100, 3418, 34400, 42100, 8527, 21300, 20100};
    auto       result    = correlationCoefficient(profits, employers);
    ASSERT_TRUE(result.has_value());
    const auto expected  = 0.05881462738716168;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance)
        << "Expected " << expected << ", got " << *result << ". Check calculation or expected value."; // Update expected value as needed
    // Alternative: test with shuffled data, or edge cases
}

// Large-scale realistic dataset: big-bank assets vs employee count
TEST(StatisticsTest, Correlation_BigBankAssets_Employees)
{
    const std::array<long double, 4> assets{
        4.4e12L,
        3.4e12L,
        2.4e12L,
        1.9e12L,
    };

    const std::array<long double, 4> employees{
        320000.0L,
        213000.0L,
        229000.0L,
        217000.0L,
    };

    auto result = correlationCoefficient(assets, employees);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    EXPECT_TRUE(std::isfinite(static_cast<double>(*result)));
    EXPECT_LE(std::abs(*result), 1.0L);
}

// Synthetic cancellation-prone dataset with large offset and small deltas.
// Disabled by default: it compares the current one-pass implementation against
// a centered two-pass reference that is numerically more stable for this shape.
TEST(StatisticsTest, DISABLED_Correlation_LargeOffsetCenteredReference)
{
    const std::array<long double, 6> x{
        1.0e12L + 1200.0L,
        1.0e12L + 1350.0L,
        1.0e12L + 1280.0L,
        1.0e12L + 1410.0L,
        1.0e12L + 1330.0L,
        1.0e12L + 1260.0L,
    };

    const std::array<long double, 6> y{
        1.0e12L + 1180.0L,
        1.0e12L + 1360.0L,
        1.0e12L + 1270.0L,
        1.0e12L + 1405.0L,
        1.0e12L + 1325.0L,
        1.0e12L + 1255.0L,
    };

    const auto centeredReference = [&]() -> long double
    {
        constexpr auto count = static_cast<long double>(x.size());

        long double meanX = 0.0L;
        long double meanY = 0.0L;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            meanX += x[i];
            meanY += y[i];
        }
        meanX /= count;
        meanY /= count;

        long double sumXX = 0.0L;
        long double sumYY = 0.0L;
        long double sumXY = 0.0L;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const auto dx = x[i] - meanX;
            const auto dy = y[i] - meanY;
            sumXX += dx * dx;
            sumYY += dy * dy;
            sumXY += dx * dy;
        }

        return sumXY / std::sqrt(sumXX * sumYY);
    }();

    auto result = correlationCoefficient(x, y);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    EXPECT_TRUE(std::isfinite(static_cast<double>(centeredReference)));
    EXPECT_LE(std::abs(centeredReference), 1.0L);
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(centeredReference), 1e-12);
}

// Test product of productTest
TEST(StatisticsTest, Product_Simple)
{
    constexpr auto productTest = std::array{1, 2, 3, 4, 5};
    auto           result      = product(productTest);
    EXPECT_EQ(result, 120);
    // Alternative: test with zeros, negatives, or large numbers
}

// Test product of insectCount
TEST(StatisticsTest, Product_InsectCount)
{
    constexpr auto insectCount = std::array{10, 1, 1000, 1, 10};
    auto           result      = product(insectCount);
    EXPECT_EQ(result, 100000);
}

//
// ---- Median tests ----
//

// Test median of sorted evenData
TEST(StatisticsTest, Median_EvenData_Sorted)
{
    constexpr std::array<mally::statlib::HighPrecisionFloat, 6> evenData = {1, 2, 3, 4, 5, 6};
    auto                                                        result   = median(evenData);
    EXPECT_EQ(result, 3.5L);
}

// Test median of sorted oddData
TEST(StatisticsTest, Median_OddData_Sorted)
{
    constexpr std::array<mally::statlib::HighPrecisionFloat, 5> oddData = {1, 2, 3, 4, 5};
    auto                                                        result  = median(oddData);
    EXPECT_EQ(result, 3.0L);
}

// Test median of unsorted data
TEST(StatisticsTest, Median_UnsortedData)
{
    const auto unsortedData = std::array{3, 1, 4, 2, 5};
    auto       result       = median(unsortedData);
    EXPECT_EQ(result, 3.0L);
}

// Test median of empty data
TEST(StatisticsTest, Median_EmptyData)
{
    const std::array<int, 0> emptyData = {};
    auto                     result    = median(emptyData);
    EXPECT_EQ(result, 0.0L);
}

// Test median single element
TEST(StatisticsTest, Median_SingleElement)
{
    const auto single = std::array{42};
    auto       result = median(single);
    EXPECT_EQ(result, 42.0L);
}

TEST(StatisticsTest, Median_VectorThreeElements)
{
    const std::vector<int> data{1, 3, 5};
    auto                   result = median(data);
    EXPECT_EQ(result, 3.0L);
}

//
// ---- Quartiles tests ----
//

// Basic quartile test for odd count
TEST(StatisticsTest, Quartiles_OddData)
{
    const std::array data{1, 2, 3, 4, 5};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 1.5L);
    EXPECT_EQ(quart.median, 3.0L);
    EXPECT_EQ(quart.q3, 4.5L);
}

// Basic quartile test for even count
TEST(StatisticsTest, Quartiles_EvenData)
{
    const std::array data{1, 2, 3, 4, 5, 6};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.0L);
    EXPECT_EQ(quart.median, 3.5L);
    EXPECT_EQ(quart.q3, 5.0L);
}

// Quartiles with unsorted input (should handle sorting internally)
TEST(StatisticsTest, Quartiles_UnsortedData)
{
    const std::array data{6, 1, 4, 2, 5, 3};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.0L);
    EXPECT_EQ(quart.median, 3.5L);
    EXPECT_EQ(quart.q3, 5.0L);
}

// Quartiles with empty range
TEST(StatisticsTest, Quartiles_EmptyData)
{
    const std::array<double, 0> data{};
    auto                        quart = quartiles(data);
    EXPECT_EQ(quart.q1, 0.0L);
    EXPECT_EQ(quart.median, 0.0L);
    EXPECT_EQ(quart.q3, 0.0L);
}

//
// ---- Summary tests ----
//

// Summary statistics on small dataset
TEST(StatisticsTest, Summary_Basic)
{
    const std::array data{12.3, 9e4, -0.6666};
    auto             summ = summary(data);

    EXPECT_NEAR(static_cast<double>(summ.min), -0.6666, 1e-9);
    EXPECT_NEAR(static_cast<double>(summ.q1), 5.8167, 1e-3);
    EXPECT_NEAR(static_cast<double>(summ.median), 12.3, 1e-9);
    EXPECT_NEAR(static_cast<double>(summ.mean), 30003.8778, 1e-3);
    EXPECT_NEAR(static_cast<double>(summ.q3), 45006.15, 1e-2);
    EXPECT_NEAR(static_cast<double>(summ.max), 90000.0, 1e-9);
}

// Empty dataset summary
TEST(StatisticsTest, Summary_Empty)
{
    const std::array<long, 0> data{};
    auto                      summ = summary(data);
    EXPECT_EQ(summ.min, 0.0L);
    EXPECT_EQ(summ.q1, 0.0L);
    EXPECT_EQ(summ.median, 0.0L);
    EXPECT_EQ(summ.mean, 0.0L);
    EXPECT_EQ(summ.q3, 0.0L);
    EXPECT_EQ(summ.max, 0.0L);
}

// Summary consistency: quartiles() vs summary()
TEST(StatisticsTest, Summary_QuartileConsistency)
{
    const std::array data{1, 2, 3, 4, 5, 6};
    auto             quart = quartiles(data);
    auto             summ  = summary(data);

    EXPECT_EQ(quart.q1, summ.q1);
    EXPECT_EQ(quart.median, summ.median);
    EXPECT_EQ(quart.q3, summ.q3);
}

// Additional tests: textbook examples and varied lengths
// 1..9 (odd count)
TEST(StatisticsTest, Quartiles_1to9)
{
    const std::array data{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.5L);
    EXPECT_EQ(quart.median, 5.0L);
    EXPECT_EQ(quart.q3, 7.5L);
}

// 1..8 (even count)
TEST(StatisticsTest, Quartiles_1to8)
{
    const std::array data{1, 2, 3, 4, 5, 6, 7, 8};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.5L);
    EXPECT_EQ(quart.median, 4.5L);
    EXPECT_EQ(quart.q3, 6.5L);
}

// Two element range
TEST(StatisticsTest, Quartiles_TwoElements)
{
    const std::array data{10, 20};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 10.0L);
    EXPECT_EQ(quart.median, 15.0L);
    EXPECT_EQ(quart.q3, 20.0L);
}

// Duplicate values
TEST(StatisticsTest, Quartiles_Duplicates)
{
    const std::array data{5, 5, 5, 5, 5, 5, 5};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 5.0L);
    EXPECT_EQ(quart.median, 5.0L);
    EXPECT_EQ(quart.q3, 5.0L);
}

// Small textbook case: 3 elements
TEST(StatisticsTest, Quartiles_ThreeElements)
{
    const std::array data{1, 3, 5};
    auto             quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.0L);
    EXPECT_EQ(quart.median, 3.0L);
    EXPECT_EQ(quart.q3, 4.0L);
}

TEST(StatisticsTest, Quartiles_VectorThreeElements)
{
    const std::vector<int> data{1, 3, 5};
    auto                   quart = quartiles(data);
    EXPECT_EQ(quart.q1, 2.0L);
    EXPECT_EQ(quart.median, 3.0L);
    EXPECT_EQ(quart.q3, 4.0L);
}

// Summary checks for varied inputs
TEST(StatisticsTest, Summary_Textbook)
{
    const std::array data{6, 7, 15, 36, 39, 40, 41};
    auto             summ = summary(data);
    EXPECT_EQ(summ.min, 6.0L);
    EXPECT_EQ(summ.q1, 7.0L);
    EXPECT_EQ(summ.median, 36.0L);
    EXPECT_NEAR(static_cast<double>(summ.mean), 26.285714285714285, 1e-9);
    EXPECT_EQ(summ.q3, 40.0L);
    EXPECT_EQ(summ.max, 41.0L);
}

TEST(StatisticsTest, Summary_EvenCount)
{
    const std::array data{1, 2, 3, 4, 5, 6, 7, 8};
    auto             summ = summary(data);
    EXPECT_EQ(summ.min, 1.0L);
    EXPECT_EQ(summ.q1, 2.5L);
    EXPECT_EQ(summ.median, 4.5L);
    EXPECT_NEAR(static_cast<double>(summ.mean), 4.5, 1e-12);
    EXPECT_EQ(summ.q3, 6.5L);
    EXPECT_EQ(summ.max, 8.0L);
}

TEST(StatisticsTest, Summary_VectorThreeElements)
{
    const std::vector<int> data{1, 3, 5};
    auto                   summ = summary(data);
    EXPECT_EQ(summ.min, 1.0L);
    EXPECT_EQ(summ.q1, 2.0L);
    EXPECT_EQ(summ.median, 3.0L);
    EXPECT_EQ(summ.mean, 3.0L);
    EXPECT_EQ(summ.q3, 4.0L);
    EXPECT_EQ(summ.max, 5.0L);
}
