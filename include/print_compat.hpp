#pragma once

// ---- feature detection -------------------------------------------------------
#ifdef __has_include
#if __has_include(<format>)
#include <version>
#endif
#endif

#ifdef __has_include
#if __has_include(<print>)
#include <version>
#endif
#endif

#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) &&                           \
    (__cpp_lib_format >= 201907L)
#define STAT_HAS_STD_PRINT 1
#else
#define STAT_HAS_STD_PRINT 0
#endif

// ---- portable API ------------------------------------------------------------
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#include <format>
#endif
#if STAT_HAS_STD_PRINT
#include <print>
#endif

// Always include fmt as the fallback backend
// Suppress GCC 15 false positive: tautological-compare in fmt/ranges.h template metaprogramming
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
#endif
#include <fmt/core.h>
#include <fmt/ranges.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace mally::statlib
{

// std::format (C++20) is detected by __cpp_lib_format
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
inline constexpr bool kHasStdFormat = true;
#else
inline constexpr bool kHasStdFormat = false;
#endif

// Portable print/println -------------------------------------------------------
template <class... Args>
inline void print(
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::format_string<Args...> fmt_str,
#else
    fmt::format_string<Args...> fmt_str,
#endif
    Args&&... args)
{
#if STAT_HAS_STD_PRINT
    std::print(fmt_str, std::forward<Args>(args)...);
#else
    fmt::print(fmt_str, std::forward<Args>(args)...);
#endif
}

template <class... Args>
inline void println(
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::format_string<Args...> fmt_str,
#else
    fmt::format_string<Args...> fmt_str,
#endif
    Args&&... args)
{
#if STAT_HAS_STD_PRINT
    std::println(fmt_str, std::forward<Args>(args)...);
#else
    fmt::println(fmt_str, std::forward<Args>(args)...);
#endif
}

// Portable format() → std::string ---------------------------------------------
template <class... Args>
inline auto format(
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::format_string<Args...> fmt_str,
#else
    fmt::format_string<Args...> fmt_str,
#endif
    Args&&... args) -> std::string
{
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    return std::format(fmt_str, std::forward<Args>(args)...);
#else
    return fmt::format(fmt_str, std::forward<Args>(args)...);
#endif
}

} // namespace mally::statlib
