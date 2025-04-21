#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include <common.h>
#include "benchmark_helper.h"
#include "../external-tools/create_copy.h"
#include "../utils/cli_helper.h"
#include "benchmark.hpp"

std::filesystem::path create_subdir(const std::filesystem::path& base_dir, const std::string& subdir_name) {
    std::filesystem::path subdir_path = base_dir / subdir_name;
    std::filesystem::create_directories(subdir_path);
    return subdir_path;
}

void save_image(cv::Mat img, const std::filesystem::path& output_dir,
                const std::filesystem::path& output_img_path_fname, const std::string& prefix) {
    char* output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/%s%s", output_dir.c_str(), prefix.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, img);
    printf("output image: %s\n", output_img_fname);
    free(output_img_fname);
}

json parse_json_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("could not open file '" + filepath + "'");
    }

    try {
        return json::parse(file);
    } catch (const json::exception& e) {
        throw std::runtime_error("could not json parse file '" + filepath + "': " + e.what());
    }
}

bool generate_single_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                          const std::string& copy_name, bool is_benchmark, std::ofstream* benchmark_csv) {
    bool success = false;
    if (is_benchmark && benchmark_csv != nullptr) {
        BenchmarkGuard benchmark_guard(copy_name, benchmark_csv);
        success = create_copy(style_params, marker_config, copy_name);
    } else {
        success = create_copy(style_params, marker_config, copy_name);
    }

    if (!success) {
        std::cerr << "Failed to generate " << copy_name << std::endl;
    } else {
        std::cout << "Generated " << copy_name << std::endl;
    }

    return success;
}

bool generate_copies(const std::map<std::string, Config>& config, const CopyStyleParams& style_params) {
    std::ofstream dummy_csv;
    return generate_copies(config, style_params, false, dummy_csv);
}

bool generate_copies(const std::map<std::string, Config>& config, const CopyStyleParams& style_params,
                     bool is_benchmark, std::ofstream& benchmark_csv) {
    auto marker_config_id = std::get<int>(config.at("marker-config").value);
    auto nb_copies = std::get<int>(config.at("nb-copies").value);
    auto warmup_iterations = std::get<int>(config.at("warmup-iterations").value);

    std::filesystem::path copies_dir = "copies";
    if (std::filesystem::exists(copies_dir)) {
        std::cout << "Cleaning existing copies directory..." << std::endl;
        std::filesystem::remove_all(copies_dir);
    }
    std::filesystem::create_directories(copies_dir);
    CopyMarkerConfig marker_config = CopyMarkerConfig::getConfigById(marker_config_id);

    bool all_success = true;
    int total_iterations = warmup_iterations + nb_copies;

    if (is_benchmark) {
        if (warmup_iterations > 0) {
            std::cout << "Starting benchmark with " << warmup_iterations << " warm-up iterations and " << nb_copies
                      << " measured iterations" << std::endl;
        } else {
            std::cout << "Starting benchmark with " << nb_copies << " iterations" << std::endl;
        }
    } else {
        if (warmup_iterations > 0) {
            std::cout << "Generating " << total_iterations << " copies (" << warmup_iterations << " warm-up + "
                      << nb_copies << " benchmark)..." << std::endl;
        } else {
            std::cout << "Generating " << nb_copies << " copies..." << std::endl;
        }
    }

    for (int i = 1; i <= total_iterations; i++) {
        bool is_warmup = i <= warmup_iterations;
        std::ostringstream copy_name;
        copy_name << "copy" << std::setw(2) << std::setfill('0') << i;

        if (is_benchmark && is_warmup) {
            std::cout << "Warmup iteration " << i << "/" << warmup_iterations << " generating: " << copy_name.str()
                      << std::endl;
        } else if (is_benchmark) {
            std::cout << "Benchmark iteration " << (i - warmup_iterations) << "/" << nb_copies
                      << " generating: " << copy_name.str() << std::endl;
        }

        bool should_measure = is_benchmark && !is_warmup;
        bool success =
            generate_single_copy(style_params, marker_config, copy_name.str(), should_measure, &benchmark_csv);
        if (!success) {
            all_success = false;
        }
    }

    if (is_benchmark) {
        std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
                  << " measured iterations." << std::endl;
    }

    return all_success;
}

BenchmarkSetup prepare_benchmark_directories(const std::map<std::string, Config>& config, 
                                           bool include_success_column, 
                                           bool create_subimg_dir) {
    BenchmarkSetup setup;
    
    // Création et nettoyage des répertoires de sortie
    setup.output_dir = std::filesystem::path{ std::get<std::string>(config.at("output-dir").value) };
    if (std::filesystem::exists(setup.output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(setup.output_dir);
    }
    std::filesystem::create_directories(setup.output_dir);
    
    // Création des sous-répertoires
    if (create_subimg_dir) {
        setup.subimg_output_dir = create_subdir(setup.output_dir, "subimg");
    }
    setup.csv_output_dir = create_subdir(setup.output_dir, "csv");
    
    // Préparation du fichier CSV pour les résultats
    std::filesystem::path benchmark_csv_path = setup.csv_output_dir / "benchmark_results.csv";
    setup.benchmark_csv.open(benchmark_csv_path);
    if (setup.benchmark_csv.is_open()) {
        if (include_success_column) {
            setup.benchmark_csv << "File,Time(ms),Success" << std::endl;
        } else {
            setup.benchmark_csv << "File,Time(ms)" << std::endl;
        }
    }
    
    return setup;
}
