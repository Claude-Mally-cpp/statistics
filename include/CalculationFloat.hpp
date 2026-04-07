/// @file CalculationFloat.hpp
/// @brief Calculation precision policy for the statistics library.
#pragma once

#include <expected>
#include <string>

namespace mally::statlib
{

/// @brief Floating-point type used for calculations and public result values.
/// @note Defaults to `double`; selected algorithms can widen later if evidence justifies it.
using CalculationFloat = double;

/// @brief Result type for computations that may fail with a textual diagnostic.
using CalculationResult = std::expected<CalculationFloat, std::string>;

} // namespace mally::statlib
