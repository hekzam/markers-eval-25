#include <iostream>
#include <fstream>
#include <map>
#include <variant>
#include <filesystem>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "generation_time.h"

bool generation_constraint(const std::map<std::string, Config>& config) {
    if (std::get<std::string>(config.at("output-dir").value).empty()) {
        std::cerr << "Output directory must not be empty" << std::endl;
        return false;
    }

    if (std::get<int>(config.at("nb-copies").value) <= 0 || std::get<int>(config.at("nb-copies").value) > 100) {
        std::cerr << "Number of copies must be between 1 and 100" << std::endl;
        return false;
    }

    if (std::get<int>(config.at("marker-config").value) < 0) {
        std::cerr << "Marker configuration must be a positive number" << std::endl;
        return false;
    }

    return true;
}

void generation_benchmark(const std::map<std::string, Config>& config) {
    if (!generation_constraint(config)) {
        return;
    }
    auto encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
    auto unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
    auto grey_level = std::get<int>(config.at("grey-level").value);
    auto dpi = std::get<int>(config.at("dpi").value);
    CopyStyleParams style_params =
        CopyStyleParams(encoded_marker_size, unencoded_marker_size, 7, 2, 5, grey_level, dpi);

    // Création et nettoyage des répertoires de sortie
    std::filesystem::path output_dir{ std::get<std::string>(config.at("output-dir").value) };
    if (std::filesystem::exists(output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(output_dir);
    }
    std::filesystem::create_directories(output_dir);
    std::filesystem::path csv_output_dir = create_subdir(output_dir, "csv");

    std::filesystem::path benchmark_csv_path = csv_output_dir / "benchmark_results.csv";
    std::ofstream benchmark_csv(benchmark_csv_path);
    if (benchmark_csv.is_open()) {
        benchmark_csv << "File,Time(ms)" << std::endl;
    }

    // Lancement du benchmark
    bool success = generate_copies(config, style_params, true, benchmark_csv);

    if (!success) {
        std::cerr << "Error: Failed to generate some or all copies." << std::endl;
    }

    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}
