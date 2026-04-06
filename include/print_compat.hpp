#pragma once

/// @file print_compat.hpp
/// @brief Portable formatting and printing helpers that prefer the standard library and fall back to `fmt`.

// ---- feature detection -------------------------------------------------------
#include <string>
#ifdef __has_include
#if __has_include(<format>)
#include <version>
#endif
#endif

// ---- portable API ------------------------------------------------------------
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#include <format>
#endif
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#include <print>
#endif

// Always include fmt as the fallback backend
// Suppress GCC 15 false positive: tautological-compare in fmt/ranges.h template metaprogramming
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
#endif
#include <fmt/core.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace mally::statlib
{

/// @brief Whether `std::format` support is available.
#if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
inline constexpr bool kHasStdFormat = true;
#else
inline constexpr bool kHasStdFormat = false;
#endif

/// @brief Whether `std::print` / `std::println` support is available.
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
inline constexpr bool kHasStdPrint = true;
#else
inline constexpr bool kHasStdPrint = false;
#endif

/// @brief Format-string type matched to the active print backend.
/// @tparam Args Argument types accepted by the format string.
template <class... Args>
using PrintFormatString =
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::format_string<Args...>;
#else
    fmt::format_string<Args...>;
#endif

/// @brief Print formatted text without a trailing newline.
/// @tparam Args Argument types.
/// @param fmt_str Format string compatible with the active backend.
/// @param args Format arguments.
template <class... Args> inline void print(PrintFormatString<Args...> fmt_str, Args&&... args)
{
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::print(fmt_str, std::forward<Args>(args)...);
#else
    fmt::print(fmt_str, std::forward<Args>(args)...);
#endif
}

/// @brief Print formatted text followed by a newline.
/// @tparam Args Argument types.
/// @param fmt_str Format string compatible with the active backend.
/// @param args Format arguments.
template <class... Args> inline void println(PrintFormatString<Args...> fmt_str, Args&&... args)
{
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
    std::println(fmt_str, std::forward<Args>(args)...);
#else
    fmt::println(fmt_str, std::forward<Args>(args)...);
#endif
}

/// @brief Format arguments into a `std::string`.
/// @tparam Args Argument types.
/// @param fmt_str Format string compatible with the active backend.
/// @param args Format arguments.
/// @return Formatted string.
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
