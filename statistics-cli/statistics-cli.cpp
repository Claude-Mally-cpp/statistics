#include <CLI/CLI.hpp>
#include <vector>
#include <string>
#include <print>
#include <statistics.hpp>

int main(int argc, char** argv) {
    CLI::App app{"Hobby Statistics CLI"};
    app.require_subcommand(1); // one subcommand required

    // ---- subcommand: summary ----
    auto* summary = app.add_subcommand(
        "summary", "Compute summary statistics for a vector of numbers");

    // Accept: --data "12.3,9e4,-0.6666"
    std::vector<double> data;
    summary->add_option("-d,--data", data,
                        "Comma-separated numeric values (e.g. \"12.3,9e4,-0.6666\")")
           ->required()
           ->delimiter(',');

    CLI11_PARSE(app, argc, argv);

    if (summary->parsed()) {
        auto mean = mally::statlib::average(data);
        auto median  = mally::statlib::median(data);
        std::println("mean:={} median={}", mean, median);
        std::println("Parsed data {} .", data);
        // TODO: compute & print the R-like table:
        // Min., 1st Qu., Median, Mean, 3rd Qu., Max.
    }

    return 0;
}
