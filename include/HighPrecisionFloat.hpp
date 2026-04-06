/// @file HighPrecisionFloat.hpp
/// @brief High-precision floating-point type and conversions for statistics library.
#pragma once
#include <expected>
#include <string>
#include <type_traits>

namespace mally::statlib
{
/// @brief Internal floating-point type used for numeric accumulation and results.
/// @note Currently aliased to `long double`.
using HighPrecisionFloat  = long double;

/// @brief Result type for computations that may fail with a textual diagnostic.
using HighPrecisionResult = std::expected<HighPrecisionFloat, std::string>;

/// @brief Pass through an existing high-precision value unchanged.
/// @param value Value already represented as `HighPrecisionFloat`.
/// @return The same value.
constexpr auto toHPF(HighPrecisionFloat value) noexcept -> HighPrecisionFloat
{
    return value;
}

/// @brief Convert an arithmetic scalar to `HighPrecisionFloat`.
/// @tparam T Arithmetic input type.
/// @param value Input value.
/// @return `value` converted to `HighPrecisionFloat`.
template <class T>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr auto toHPF(T value) noexcept -> HighPrecisionFloat
{
    return static_cast<HighPrecisionFloat>(value);
}

/// @brief Deleted overload to prevent accidental conversion of container-like types.
/// @tparam R Type with a nested `value_type`.
template <class R>
    requires requires { typename R::value_type; }
auto toHPF(const R&) -> HighPrecisionFloat = delete;

} // namespace mally::statlib
