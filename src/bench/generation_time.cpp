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

    // Préparation des répertoires et du fichier CSV
    BenchmarkSetup benchmark_setup = prepare_benchmark_directories(config, false);
    std::ofstream& benchmark_csv = benchmark_setup.benchmark_csv;

    generate_copies(config, style_params, true, benchmark_csv);

    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}