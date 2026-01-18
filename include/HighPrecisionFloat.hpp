/// @file HighPrecisionFloat.hpp
/// @brief High-precision floating-point type and conversions for statistics library.
#pragma once
#include <concepts>
#include <expected>
#include <string>
#include <type_traits>

namespace mally::statlib
{
// TODO: reconsider always using high precision internally.
using HighPrecisionFloat  = long double;
using HighPrecisionResult = std::expected<HighPrecisionFloat, std::string>;

// Pass-through for HPF
constexpr auto toHPF(HighPrecisionFloat v) noexcept -> HighPrecisionFloat
{
    return v;
}

// Accept only arithmetic scalars (prevents containers from instantiating)
template <class T>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr auto toHPF(T v) noexcept -> HighPrecisionFloat
{
    return static_cast<HighPrecisionFloat>(v);
}

// Hard error if something “container-like” (has value_type) calls toHPF by mistake
template <class R>
    requires requires { typename R::value_type; }
auto toHPF(const R&) -> HighPrecisionFloat = delete;

} // namespace mally::statlib
