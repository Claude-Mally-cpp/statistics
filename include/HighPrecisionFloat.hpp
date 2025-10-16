
#include <concepts>
#include <type_traits>

namespace mally::statlib 
{

using HighPrecisionFloat  = long double;
using HighPrecisionResult = std::expected<HighPrecisionFloat, std::string>;

// keep a pass-through for HPF
constexpr HighPrecisionFloat toHPF(HighPrecisionFloat v) noexcept { return v; }

// accept any arithmetic scalar
template <class T>
    requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr HighPrecisionFloat toHPF(T v) noexcept {
    return static_cast<HighPrecisionFloat>(v);
}

template <class R>
    requires requires { typename R::value_type; } // crude "looks like container"
HighPrecisionFloat toHPF(const R&) = delete;

/// @brief Complete summary of numeric range (R-style output)
struct SummaryStats
{
    HighPrecisionFloat min;
    HighPrecisionFloat q1;
    HighPrecisionFloat median;
    HighPrecisionFloat mean;
    HighPrecisionFloat q3;
    HighPrecisionFloat max;
};


} // namespace mally::statlib



// Formatter for SummaryStats
// We parse ONE float spec and reuse it for each field so users can write
// std::format("{:.3f}", stats) and have every number use .3f.
template <> struct std::formatter<mally::statlib::SummaryStats>
{
    // Reuse the standard float formatter to parse/format the numbers
    std::formatter<long double> elem_;

    // Parse the *element* spec from the full spec. Whatever the caller puts
    // after ':' (precision, type, width, etc.) applies to each field.
    constexpr auto parse(std::format_parse_context& pc)
    {
        return elem_.parse(pc);
    }

    auto format(const mally::statlib::SummaryStats& s, std::format_context& ctx) const
    {
        auto out = ctx.out();

        out = std::format_to(out, "SummaryStats{{min=");
        out = elem_.format(static_cast<long double>(s.min), ctx);

        out = std::format_to(out, ", q1=");
        out = elem_.format(static_cast<long double>(s.q1), ctx);

        out = std::format_to(out, ", median=");
        out = elem_.format(static_cast<long double>(s.median), ctx);

        out = std::format_to(out, ", mean=");
        out = elem_.format(static_cast<long double>(s.mean), ctx);

        out = std::format_to(out, ", q3=");
        out = elem_.format(static_cast<long double>(s.q3), ctx);

        out = std::format_to(out, ", max=");
        out = elem_.format(static_cast<long double>(s.max), ctx);

        out = std::format_to(out, "}}");
        return out;
    }
};

/*
int main()
{
    SummaryStats stats{HighPrecisionFloat{1.0L},
                       HighPrecisionFloat{2.0L},
                       HighPrecisionFloat{3.141592653589793L},
                       HighPrecisionFloat{2.718281828459045L},
                       HighPrecisionFloat{4.0L},
                       HighPrecisionFloat{5.0L}};

    // Default formatting (like {:g})
    std::println("{}", stats);

    // Uniform precision across all fields
    std::println("{:.3f}", stats);

    // Alignment/width + scientific notation for each number
    std::println("{:>80.6e}", stats);

    // You can also format the element type directly:
    std::println("mean = {:.8f}", stats.mean);
}
*/
