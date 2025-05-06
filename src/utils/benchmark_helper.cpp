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

BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column,
                                             bool create_subimg_dir, bool write_header) {
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

    std::filesystem::path benchmark_csv_path = setup.csv_output_dir / "benchmark_results.csv";
    setup.benchmark_csv.open(benchmark_csv_path);
    if (setup.benchmark_csv.is_open() && write_header) {
        if (include_success_column) {
            setup.benchmark_csv << "File,Time(ms),Success" << std::endl;
        } else {
            setup.benchmark_csv << "File,Time(ms)" << std::endl;
        }
    }

    return setup;
}
