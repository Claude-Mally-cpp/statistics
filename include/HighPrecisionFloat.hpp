#pragma once
#include <concepts>
#include <type_traits>
#include <expected>
#include <string>

namespace mally::statlib {

using HighPrecisionFloat  = long double;
using HighPrecisionResult = std::expected<HighPrecisionFloat, std::string>;

// Pass-through for HPF
constexpr HighPrecisionFloat toHPF(HighPrecisionFloat v) noexcept { return v; }

// Accept only arithmetic scalars (prevents containers from instantiating)
template <class T>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr HighPrecisionFloat toHPF(T v) noexcept {
    return static_cast<HighPrecisionFloat>(v);
}

// Hard error if something “container-like” (has value_type) calls toHPF by mistake
template <class R>
    requires requires { typename R::value_type; }
HighPrecisionFloat toHPF(const R&) = delete;

} // namespace mally::statlib
