// test_statistics.cpp
#include <gtest/gtest.h>
#include "statistics.hpp"
#include "print_compat.hpp"

// Test correlation between returnsA and returnsB
TEST(StatisticsTest, Correlation_AB) {
    const std::array<double, 3> returnsA = {0.07, 0.09, 0.10};
    const std::array<double, 3> returnsB = {0.085, 0.07, 0.095};
    auto result = mally::statlib::correlationCoefficient(returnsA, returnsB);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    // Use fixed expected value, update if implementation changes
    mally::statlib::HighPrecisionFloat expected = 0.21677749238102959;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance) << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
    // Alternative: test for range, or parameterize with more datasets
}

// Test correlation between returnsA and returnsC
TEST(StatisticsTest, Correlation_AC) {
    const auto returnsA = std::array<double, 3>{0.07, 0.09, 0.10};
    const auto returnsC = std::array<double, 3>{0.12, 0.11, 0.10};
    auto result = mally::statlib::correlationCoefficient(returnsA, returnsC);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    mally::statlib::HighPrecisionFloat expected = -0.9819805060619121;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance) << "Expected " << -0.981980 << ", got " << *result << ". Check calculation or expected value.";
}

// Test correlation between returnsB and returnsC
TEST(StatisticsTest, Correlation_BC) {
    const auto returnsB = std::array{0.085, 0.07, 0.095};
    const auto returnsC = std::array{0.12, 0.11, 0.10};
    auto result = mally::statlib::correlationCoefficient(returnsB, returnsC);
    ASSERT_TRUE(result.has_value()) << "Failed to compute correlation: " << result.error();
    mally::statlib::HighPrecisionFloat expected = -0.39735970711947155;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance) << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
}

// Test covariance between rendemetsTitresX and rendementsMarche
TEST(StatisticsTest, Covariance_TitresX_Marche) {
    constexpr auto returnsX = std::array{-0.10, -0.05, 0.00, 0.08, 0.14, 0.20, 0.25};
    constexpr auto marketReturns = std::array{-0.20, -0.10, -0.05, 0.00, 0.10, 0.20, 0.30};
    auto result = mally::statlib::covariance(returnsX, marketReturns);
    ASSERT_TRUE(result.has_value());
    mally::statlib::HighPrecisionFloat expected = 0.022571428571428576;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance) << "Expected " << expected << ", got " << *result << ". Check calculation or expected value.";
    // Alternative: test for symmetry, or with randomized data
}

// Test correlation between profits and employers
TEST(StatisticsTest, Correlation_Profits_Employers) {
    const auto profits =
        std::array{300, 9300, 20900, 31000, 41400, 47700, 60800, 79500, 80400, 89000,
                   118300, 119700, 153000, 252800, 333300, 412000, 424300, 454000, 829000, 86500,
                   176000, 227400, 471300, 681100, 747000, 859800, 939500, 1082000, 1102200, 1495400};
    const auto employers =
        std::array{7523, 8200, 12068, 9500, 5000, 18000, 4708, 13740, 95000, 8200,
                   56000, 31404, 8578, 2900, 9100, 10200, 9548, 82300, 28334, 40929,
                   50816, 54100, 28200, 83100, 3418, 34400, 42100, 8527, 21300, 20100};
    auto result = mally::statlib::correlationCoefficient(profits, employers);
    ASSERT_TRUE(result.has_value());
    const auto expected = 0.05881462738716168;
    const auto tolerance = 1e-10;
    EXPECT_NEAR(static_cast<double>(*result), static_cast<double>(expected), tolerance ) << "Expected " << expected << ", got " << *result << ". Check calculation or expected value."; // Update expected value as needed
    // Alternative: test with shuffled data, or edge cases
}

// Test product of productTest
TEST(StatisticsTest, Product_Simple) {
    constexpr auto productTest = std::array{1, 2, 3, 4, 5};
    auto result = mally::statlib::product(productTest);
    EXPECT_EQ(result, 120);
    // Alternative: test with zeros, negatives, or large numbers
}

// Test product of insectCount
TEST(StatisticsTest, Product_InsectCount) {
    constexpr auto insectCount = std::array{10, 1, 1000, 1, 10};
    auto result = mally::statlib::product(insectCount);
    EXPECT_EQ(result, 100000);
#   if 0
    // To test warnings
    const auto testFloatConversion = 42.0000000042;
    int dangerousConversion = testFloatConversion;
    std::println("float to int conversion: {} -> {}", testFloatConversion, dangerousConversion);
#   endif
}
