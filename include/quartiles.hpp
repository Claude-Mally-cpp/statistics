#pragma once
#include "HighPrecisionFloat.hpp"

#include <array>
#include <algorithm>
#include <numeric>
#include <format>
#include <type_traits>
#include <ranges>

namespace mally::statlib {

// ----------------------------- Data types -----------------------------

struct QuartileSummary {
    HighPrecisionFloat q1{};
    HighPrecisionFloat median{};
    HighPrecisionFloat q3{};
};

struct SummaryStats {
    std::size_t        count{};  // N
    HighPrecisionFloat min{};    // minimum
    HighPrecisionFloat q1{};     // Tukey lower hinge
    HighPrecisionFloat median{}; // median
    HighPrecisionFloat mean{};   // arithmetic mean
    HighPrecisionFloat q3{};     // Tukey upper hinge
    HighPrecisionFloat max{};    // maximum
};

// ----------------------------- Internals -----------------------------

namespace detail {

// Median of a *sorted* inclusive slice [lo..hi] within one array
template <std::size_t N>
constexpr HighPrecisionFloat medianSortedSlice(const std::array<HighPrecisionFloat, N>& a,
                                               std::size_t lo, std::size_t hi)
{
    const std::size_t len = (hi >= lo) ? (hi - lo + 1) : 0;
    if (len == 0) return 0.0L;

    if (len & 1U) { // odd
        return a[lo + len / 2];
    } else {        // even
        const std::size_t mid_hi = lo + len / 2;
        const std::size_t mid_lo = mid_hi - 1;
        return (a[mid_lo] + a[mid_hi]) / 2.0L;
    }
}

template <std::size_t N>
constexpr HighPrecisionFloat medianSorted(const std::array<HighPrecisionFloat, N>& a)
{
    if constexpr (N == 0) {
        return 0.0L;
    } else if constexpr (N % 2 == 1) {
        return a[N / 2];
    } else {
        return (a[N / 2 - 1] + a[N / 2]) / 2.0L;
    }
}

} // namespace detail

// ----------------------------- Quartiles (array only) -----------------------------

/// @brief Compute Q1, Median, Q3 assuming the input range is already sorted.
/// @note Tukey hinges on a *sorted* array<HPF, N> (median included in halves when N is odd)
/// @param range Input sorted range of numbers.
/// @return QuartileSummary with Q1, Median, Q3.
/// @details This function computes the quartiles of a sorted range of numbers.
// median of *sorted* whole array
template <std::size_t N>
constexpr HighPrecisionFloat medianSorted(const std::array<HighPrecisionFloat, N>& a)
{
    if constexpr (N == 0) {
        return 0.0L;
    } else if constexpr (N % 2 == 1) {
        return a[N / 2];
    } else {
        return (a[N / 2 - 1] + a[N / 2]) / 2.0L;
    }
}

// Tukey hinges on a *sorted* array<HPF, N>
template <std::size_t N>
constexpr QuartileSummary sortedQuartiles(const std::array<HighPrecisionFloat, N>& sorted)
{
    if constexpr (N == 0) {
        return {};
    } else {
        const HighPrecisionFloat med = medianSorted(sorted);

        std::size_t loL = 0, loH, hiL, hiH = N - 1;

        if constexpr (N % 2 == 1) {
            const std::size_t mid = N / 2;
            loH = mid;   // [0 .. mid]
            hiL = mid;   // [mid .. N-1]
        } else {
            loH = N / 2 - 1; // [0 .. N/2 - 1]
            hiL = N / 2;     // [N/2 .. N-1]
        }

        const HighPrecisionFloat q1 = detail::medianSortedSlice(sorted, loL, loH);
        const HighPrecisionFloat q3 = detail::medianSortedSlice(sorted, hiL, hiH);
        return {q1, med, q3};
    }
}

// quartiles for std::array<*, N> (no vectors involved)
// Accepts arrays of arithmetic or HighPrecisionFloat; converts element-wise to HPF,
// sorts a copy (still std::array), then computes Tukey hinges.
template <class T, std::size_t N>
    requires (std::is_arithmetic_v<std::remove_cvref_t<T>> ||
              std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr QuartileSummary quartiles(const std::array<T, N>& data)
{
    if constexpr (N == 0) return {};

    std::array<HighPrecisionFloat, N> hp{};
    for (std::size_t i = 0; i < N; ++i) hp[i] = toHPF(data[i]);

    std::ranges::sort(hp);
    return sortedQuartiles(hp);
}

// ----------------------------- Summary (array only) -----------------------------

// summary(array) => {N, min, Q1, median, mean, Q3, max}
template <class T, std::size_t N>
    requires (std::is_arithmetic_v<std::remove_cvref_t<T>> ||
              std::is_same_v<std::remove_cvref_t<T>, HighPrecisionFloat>)
constexpr SummaryStats summary(const std::array<T, N>& data)
{
    SummaryStats out{};
    out.count = N;

    if constexpr (N == 0) {
        return out; // leave zeros
    }

    // Convert to HPF and sort (still array, no vectors)
    std::array<HighPrecisionFloat, N> hp{};
    for (std::size_t i = 0; i < N; ++i) hp[i] = toHPF(data[i]);
    std::ranges::sort(hp);

    // Min/Max (after sort)
    out.min = hp.front();
    out.max = hp.back();

    // Quartiles (on sorted hp)
    const auto q = sortedQuartiles(hp);
    out.q1     = q.q1;
    out.median = q.median;
    out.q3     = q.q3;

    // Mean (use accumulation in HPF)
    HighPrecisionFloat acc = 0.0L;
    for (const auto& v : hp) acc += v;
    out.mean = acc / static_cast<HighPrecisionFloat>(N);

    return out;
}

} // namespace mally::statlib

// ----------------------------- std::formatter for SummaryStats -----------------------------
template <>
struct std::formatter<mally::statlib::SummaryStats, char>
{
    // Reuse the same numeric spec for each number (precision, width, sci, etc.)
    std::formatter<mally::statlib::HighPrecisionFloat, char> num_{};

    constexpr auto parse(std::format_parse_context& ctx)
    {
        // Propagate user's spec (e.g. ".3f", ">12.6e", etc.)
        return num_.parse(ctx);
    }

    template <class FormatContext>
    auto format(const mally::statlib::SummaryStats& s, FormatContext& ctx) const
    {
        using std::format_to;
        format_to(ctx.out(), "n={} min=", s.count); num_.format(s.min,    ctx);
        format_to(ctx.out(), " q1=");               num_.format(s.q1,     ctx);
        format_to(ctx.out(), " median=");           num_.format(s.median, ctx);
        format_to(ctx.out(), " mean=");             num_.format(s.mean,   ctx);
        format_to(ctx.out(), " q3=");               num_.format(s.q3,     ctx);
        format_to(ctx.out(), " max=");              num_.format(s.max,    ctx);
        return ctx.out();
    }
};
