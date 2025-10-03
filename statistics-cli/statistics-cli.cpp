#include <CLI/CLI.hpp>
#include <vector>
#include <string>
#include <print>

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

    // (Optional) allow whitespace around commas like "1, 2, 3"
    // CLI11 ignores spaces by default when splitting on delimiter.

    CLI11_PARSE(app, argc, argv);

    // --- stub: we'll compute later; for now just confirm parsing ---
    if (summary->parsed()) {
        std::println("Parsed {} value(s) for summary.", data.size());
        // TODO: compute & print the R-like table:
        // Min., 1st Qu., Median, Mean, 3rd Qu., Max.
    }

    return 0;
}
