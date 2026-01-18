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
template <class OutIt> auto formatSummaryCore(const SummaryStats& s, OutIt out) -> OutIt
{
    return fmt::format_to(out, "n={}, min={}, q1={}, median={}, q3={}, max={}, mean={}", s.count, s.min, s.q1, s.median,
                          s.q3, s.max, s.mean);
}

} // namespace mally::statlib

// ---- std::format specialization (when available) -----------------------------
#if STAT_HAS_STD_FORMAT
#include <format>
template <> struct std::formatter<mally::statlib::SummaryStats, char>
{
    // Only "{}" supported for now; extend parse() if you add specifiers.
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    auto format(const mally::statlib::SummaryStats& s, std::format_context& ctx) const
    {
        return mally::statlib::formatSummaryCore(s, ctx.out());
    }
};
#else
// ---- fmt::formatter fallback -------------------------------------------------
template <> struct fmt::formatter<mally::statlib::SummaryStats>
{
    static constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <class FormatContext> auto format(const mally::statlib::SummaryStats& s, FormatContext& ctx) const
    {
        return mally::statlib::formatSummaryCore(s, ctx.out());
    }
};
#endif
