#include <CLI/CLI.hpp>
#include <print>
#include <statistics.hpp>
#include <string>
#include <vector>

#include "print_compat.hpp"
#include "statistics_version.hpp"

int main(int argc, char** argv)
{
    CLI::App app{"Hobby Statistics CLI"};
    app.set_version_flag("-V,--version", std::string{"statistics "} + std::string{mally::statlib::version});

    // ---- subcommand: summary ----
    auto* summary = app.add_subcommand("summary", "Compute summary statistics for a vector of numbers");

    // Accept: --data "12.3,9e4,-0.6666"
    std::vector<mally::statlib::CalculationFloat> data;
    summary->add_option("-d,--data", data, "Comma-separated numeric values (e.g. \"12.3,9e4,-0.6666\")")->required()->delimiter(',');

    if (argc == 1)
    {
        print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);
    if (summary->parsed())
    {
        auto summaryData = mally::statlib::summary(data);
        println("Computed summary: {}", summaryData);
    }

    return 0;
}
