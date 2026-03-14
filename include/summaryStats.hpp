/// @file summaryStats.hpp
/// @brief Summary statistics structure and formatter specializations.
#pragma once

#include "HighPrecisionFloat.hpp"
#include "print_compat.hpp"
#include <cstddef>
#include <string>

namespace mally::statlib
{

/// @brief Summary of basic descriptive statistics.
struct SummaryStats
{
    std::size_t        count{};  ///< @brief Number of elements.
    HighPrecisionFloat min{};    ///< @brief Minimum value.
    HighPrecisionFloat q1{};     ///< @brief First quartile (Tukey lower hinge).
    HighPrecisionFloat median{}; ///< @brief Median.
    HighPrecisionFloat mean{};   ///< @brief Arithmetic mean.
    HighPrecisionFloat q3{};     ///< @brief Third quartile (Tukey upper hinge).
    HighPrecisionFloat max{};    ///< @brief Maximum value.

    /// @todo Add standard deviation and variance?

    [[nodiscard]] auto toString() const -> std::string
    {
        return format("n={}, min={}, q1={}, median={}, q3={}, max={}, mean={}", count, min, q1, median, q3, max, mean);
    }
};

// ---- shared core for both std::format and fmt::format ------------------------
template <class OutIt> auto formatSummaryCore(const SummaryStats& summary, OutIt out) -> OutIt
{
    return fmt::format_to(out, "n={}, min={}, q1={}, median={}, q3={}, max={}, mean={}", summary.count, summary.min, summary.q1, summary.median,
                          summary.q3, summary.max, summary.mean);
}

} // namespace mally::statlib

// ---- std::format specialization (when available) -----------------------------
// ---- fmt::formatter ----------------------------------------------------------
template <> struct fmt::formatter<mally::statlib::SummaryStats>
{
    static constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <class FormatContext> auto format(const mally::statlib::SummaryStats& summary, FormatContext& ctx) const
    {
        return mally::statlib::formatSummaryCore(summary, ctx.out());
    }
};

#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#include <format>
template <> struct std::formatter<mally::statlib::SummaryStats, char>
{
    // Only "{}" supported for now; extend parse() if you add specifiers.
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const mally::statlib::SummaryStats& summary, std::format_context& ctx) const
    {
        return mally::statlib::formatSummaryCore(summary, ctx.out());
    }
};
#endif
