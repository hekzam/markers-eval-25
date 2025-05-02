#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <optional>
#include <functional>

#include <common.h>
#include "benchmark_helper.h"
#include "../external-tools/create_copy.h"
#include "../utils/cli_helper.h"
#include "benchmark.hpp"
#include "csv_utils.h"

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
                          const std::string& copy_name, Csv<std::string, float, int, CopyMarkerConfig>* benchmark_csv) {
    bool success = false;
    if (benchmark_csv != nullptr) {
        BenchmarkGuard benchmark_guard(copy_name);
        success = create_copy(style_params, marker_config, copy_name);
        float time = benchmark_guard.end();
        benchmark_csv->add_row({ copy_name, time, success, marker_config });
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

bool generate_copies(int nb_copies, int warmup_iterations, const CopyStyleParams& style_params,
                     const CopyMarkerConfig& marker_config,
                     Csv<std::string, float, int, CopyMarkerConfig>* benchmark_csv) {

    std::filesystem::path copies_dir = "copies";
    if (std::filesystem::exists(copies_dir)) {
        std::cout << "Cleaning existing copies directory..." << std::endl;
        std::filesystem::remove_all(copies_dir);
    }
    std::filesystem::create_directories(copies_dir);

    bool all_success = true;
    int total_iterations = warmup_iterations + nb_copies;

    if (warmup_iterations > 0) {
        std::cout << "Starting benchmark with " << warmup_iterations << " warm-up iterations and " << nb_copies
                  << " measured iterations" << std::endl;
    } else {
        std::cout << "Starting benchmark with " << nb_copies << " iterations" << std::endl;
    }

    for (int i = 1; i <= total_iterations; i++) {
        bool is_warmup = i <= warmup_iterations;
        std::ostringstream copy_name;
        copy_name << "copy" << std::setw(2) << std::setfill('0') << i;

        if (benchmark_csv != nullptr && is_warmup) {
            std::cout << "Warmup iteration " << i << "/" << warmup_iterations << " generating: " << copy_name.str()
                      << std::endl;
        } else if (benchmark_csv != nullptr) {
            std::cout << "Benchmark iteration " << (i - warmup_iterations) << "/" << nb_copies
                      << " generating: " << copy_name.str() << std::endl;
        }

        bool should_measure = benchmark_csv != nullptr && !is_warmup;
        if (!should_measure) {
            benchmark_csv = nullptr;
        }
        bool success = generate_single_copy(style_params, marker_config, copy_name.str(), benchmark_csv);
        if (!success) {
            all_success = false;
        }
    }

    std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
              << " measured iterations." << std::endl;

    if (!all_success) {
        std::cerr << "Error: Failed to generate some or all copies." << std::endl;
    }

    return all_success;
}

BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column,
                                             bool create_subimg_dir) {
    BenchmarkSetup setup;

    setup.output_dir = std::filesystem::path{ output_dir };
    if (std::filesystem::exists(setup.output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(setup.output_dir);
    }
    std::filesystem::create_directories(setup.output_dir);

    if (create_subimg_dir) {
        setup.subimg_output_dir = create_subdir(setup.output_dir, "subimg");
    }
    setup.csv_output_dir = create_subdir(setup.output_dir, "csv");

    return setup;
}
