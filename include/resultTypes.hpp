/// @file resultTypes.hpp
/// @brief Public result-type traits for the statistics library.
#pragma once

#include <type_traits>

namespace mally::statlib
{

/// @brief Statistical result type deduced from the input value type.
/// @details Integral inputs use `double`; floating-point inputs preserve their value type.
template <class T>
using PublicResultType = std::conditional_t<std::is_floating_point_v<std::remove_cvref_t<T>>, std::remove_cvref_t<T>, double>;

namespace detail
{

/// @cond DOXYGEN_SKIP
template <class T> using CalculationType = std::conditional_t<std::is_same_v<PublicResultType<T>, float>, double, PublicResultType<T>>;
/// @endcond

} // namespace detail

} // namespace mally::statlib
