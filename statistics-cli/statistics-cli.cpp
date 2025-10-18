#include <CLI/CLI.hpp>
#include <print>
#include <statistics.hpp>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    CLI::App app{"Hobby Statistics CLI"};
    app.require_subcommand(1); // one subcommand required

    // ---- subcommand: summary ----
    auto* summary = app.add_subcommand("summary", "Compute summary statistics for a vector of numbers");

    // Accept: --data "12.3,9e4,-0.6666"
    std::vector<mally::statlib::HighPrecisionFloat> data;
    summary->add_option("-d,--data", data, "Comma-separated numeric values (e.g. \"12.3,9e4,-0.6666\")")
        ->required()
        ->delimiter(',');

    CLI11_PARSE(app, argc, argv);

    if (summary->parsed())
    {
        auto summaryData = mally::statlib::summary(data);
        std::println("Computed summary: {}", summaryData);
    }

    return 0;
}
