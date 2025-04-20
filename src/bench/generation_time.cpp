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
#include "benchmark.hpp"
#include "generation_time.h"

bool generation_constraint(std::map<std::string, Config> config) {
    if (std::get<std::string>(config["output-dir"].value).empty()) {
        std::cerr << "Output directory must not be empty" << std::endl;
        return false;
    }

    if (std::get<int>(config["nb-copies"].value) <= 0 || std::get<int>(config["nb-copies"].value) > 100) {
        std::cerr << "Number of copies must be between 1 and 100" << std::endl;
        return false;
    }

    if (std::get<int>(config["marker-config"].value) < 0) {
        std::cerr << "Marker configuration must be a positive number" << std::endl;
        return false;
    }

    return true;
}

void generation_benchmark(std::map<std::string, Config> config) {
    if (!generation_constraint(config)) {
        // print_help_config(default_config_generation_time);
        return;
    }

    int nb_copies = std::get<int>(config["nb-copies"].value);
    int warmup_iterations = std::get<int>(config["warmup-iterations"].value);
    auto encoded_marker_size = std::get<int>(config["encoded-marker_size"].value);
    auto unencoded_marker_size = std::get<int>(config["unencoded-marker_size"].value);
    auto grey_level = std::get<int>(config["grey-level"].value);
    auto dpi = std::get<int>(config["dpi"].value);
    auto marker_config_id = std::get<int>(config["marker-config"].value);

    // Création et nettoyage des répertoires de sortie
    std::filesystem::path output_dir{ std::get<std::string>(config["output-dir"].value) };
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

    std::filesystem::path copies_dir = "copies";
    if (std::filesystem::exists(copies_dir)) {
        std::cout << "Cleaning existing copies directory..." << std::endl;
        std::filesystem::remove_all(copies_dir);
    }
    std::filesystem::create_directories(copies_dir);

    CopyStyleParams style_params;
    style_params = CopyStyleParams(encoded_marker_size, unencoded_marker_size, 7, 2, 5, grey_level, dpi);

    CopyMarkerConfig marker_config = CopyMarkerConfig::getConfigById(marker_config_id);

    int total_iterations = warmup_iterations + nb_copies;
    if (warmup_iterations > 0) {
        std::cout << "Starting benchmark with " << warmup_iterations << " warm-up iterations and " << nb_copies
                  << " measured iterations" << std::endl;
    } else {
        std::cout << "Starting benchmark with " << nb_copies << " iterations" << std::endl;
    }

    // Exécution du benchmark
    for (int i = 1; i <= total_iterations; i++) {
        bool is_warmup = i <= warmup_iterations;
        std::ostringstream copy_name;
        copy_name << "copy" << std::setw(2) << std::setfill('0') << i;

        if (is_warmup) {
            std::cout << "Warmup iteration " << i << "/" << warmup_iterations << " generating: " << copy_name.str()
                      << std::endl;

            bool success = create_copy(style_params, marker_config, copy_name.str());
            std::cout << "  Result: " << (success ? "Success" : "Failed") << std::endl;
        } else {
            std::cout << "Benchmark iteration " << (i - warmup_iterations) << "/" << nb_copies 
                      << " generating: " << copy_name.str() << std::endl;
                      
            BenchmarkGuardSimple benchmark_guard(copy_name.str(), &benchmark_csv);
            bool success = create_copy(style_params, marker_config, copy_name.str());

            if (!success) {
                std::cerr << "Failed to generate " << copy_name.str() << std::endl;
            }
        }
    }

    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }

    std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
              << " measured iterations." << std::endl;
}
