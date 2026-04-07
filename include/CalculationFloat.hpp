/// @file CalculationFloat.hpp
/// @brief Public result and internal calculation type traits for the statistics library.
#pragma once

#include <expected>
#include <string>
#include <type_traits>

namespace mally::statlib
{

/// @brief Public result type follows the input type, except integral inputs promote to `double`.
template <class T>
using PublicResultType = std::conditional_t<std::is_floating_point_v<std::remove_cvref_t<T>>, std::remove_cvref_t<T>, double>;

/// @brief Internal calculation type may widen beyond the public result type for stability.
/// @note The current policy widens `float` to `double`, keeps `double` as `double`,
///       and preserves `long double`.
template <class T> using CalculationFloat = std::conditional_t<std::is_same_v<PublicResultType<T>, float>, double, PublicResultType<T>>;

/// @brief Public result wrapper for computations that may fail with a textual diagnostic.
template <class T> using CalculationResult = std::expected<PublicResultType<T>, std::string>;

} // namespace mally::statlib
