#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <filesystem>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "generation_time.h"

bool generation_constraint(const std::unordered_map<std::string, Config>& config) {
    if (std::get<std::string>(config.at("output-dir").value).empty()) {
        std::cerr << "Output directory must not be empty" << std::endl;
        return false;
    }

    if (std::get<int>(config.at("nb-copies").value) <= 0 || std::get<int>(config.at("nb-copies").value) > 100) {
        std::cerr << "Number of copies must be between 1 and 100" << std::endl;
        return false;
    }

    if (std::get<std::string>(config.at("marker-config").value).empty()) {
        std::cerr << "Marker configuration must not be empty" << std::endl;
        return false;
    }

    return true;
}

void generation_benchmark(const std::unordered_map<std::string, Config>& config) {
    if (!generation_constraint(config)) {
        return;
    }
    auto output_dir = std::get<std::string>(config.at("output-dir").value);
    auto encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
    auto unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
    auto grey_level = std::get<int>(config.at("grey-level").value);
    auto dpi = std::get<int>(config.at("dpi").value);
    auto marker_config = std::get<std::string>(config.at("marker-config").value);
    auto nb_copies = std::get<int>(config.at("nb-copies").value);
    auto warmup_iterations = std::get<int>(config.at("warmup-iterations").value);

    CopyMarkerConfig copy_marker_config;
    if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
        std::cerr << "Invalid marker configuration: " << marker_config << std::endl;
        return;
    }
    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.unencoded_marker_size = unencoded_marker_size;
    style_params.grey_level = grey_level;
    style_params.dpi = dpi;

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories(output_dir, false);
    std::ofstream& benchmark_csv = benchmark_setup.benchmark_csv;

    generate_copies(nb_copies, warmup_iterations, style_params, copy_marker_config, benchmark_csv);

    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}