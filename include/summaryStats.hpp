/// @file summaryStats.hpp
/// @brief Summary statistics structure and formatter specializations.
#pragma once

#include "print_compat.hpp"
#include <cstddef>
#include <fmt/base.h>
#include <fmt/format.h>
#include <string>

namespace mally::statlib
{

/// @brief Summary of basic descriptive statistics.
template <class T> struct SummaryStats
{
    std::size_t count{};  ///< @brief Number of elements.
    T           min{};    ///< @brief Minimum value.
    T           q1{};     ///< @brief First quartile (Tukey lower hinge).
    T           median{}; ///< @brief Median.
    T           mean{};   ///< @brief Arithmetic mean.
    T           q3{};     ///< @brief Third quartile (Tukey upper hinge).
    T           max{};    ///< @brief Maximum value.

    /// @todo Add standard deviation and variance?

    /// @brief Convert the summary to a human-readable string.
    /// @return Formatted summary string containing count, quartiles, extrema, and mean.
    [[nodiscard]] auto toString() const -> std::string
    {
        return format("n={}, min={}, q1={}, median={}, q3={}, max={}, mean={}", count, min, q1, median, q3, max, mean);
    }
};

/// @brief Shared formatting helper used by both `fmt::formatter` and `std::formatter`.
/// @tparam T Public result value type.
/// @tparam OutIt Output iterator type used by the formatting backend.
/// @param summary Summary value to format.
/// @param out Output iterator destination.
/// @return Advanced output iterator after writing the formatted representation.
template <class T, class OutIt> auto formatSummaryCore(const SummaryStats<T>& summary, OutIt out) -> OutIt
{
    return fmt::format_to(out,
                          "n={}, min={}, q1={}, median={}, q3={}, max={}, mean={}",
                          summary.count,
                          summary.min,
                          summary.q1,
                          summary.median,
                          summary.q3,
                          summary.max,
                          summary.mean);
}

} // namespace mally::statlib

/// @cond DOXYGEN_SKIP
template <class T> struct fmt::formatter<mally::statlib::SummaryStats<T>>
{
    static constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <class FormatContext> auto format(const mally::statlib::SummaryStats<T>& summary, FormatContext& ctx) const
    {
        return mally::statlib::formatSummaryCore(summary, ctx.out());
    }
};

#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#include <format>
template <class T> struct std::formatter<mally::statlib::SummaryStats<T>, char>
{
    // Only "{}" supported for now; extend parse() if you add specifiers.
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    static auto format(const mally::statlib::SummaryStats<T>& summary, std::format_context& ctx)
    {
        return mally::statlib::formatSummaryCore(summary, ctx.out());
    }
};
#endif
/// @endcond
