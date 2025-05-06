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

void clean_directory(const std::string& dir) {
    if (std::filesystem::exists(dir)) {
        std::cout << "Cleaning existing directory: " << dir << std::endl;
        std::filesystem::remove_all(dir);
    }
    std::filesystem::create_directories(dir);
}

BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column,
                                             bool create_subimg_dir) {
    BenchmarkSetup setup;

    setup.output_dir = std::filesystem::path{ output_dir };
    clean_directory(setup.output_dir);

    if (create_subimg_dir) {
        setup.subimg_output_dir = create_subdir(setup.output_dir, "subimg");
    }
    setup.csv_output_dir = create_subdir(setup.output_dir, "csv");

    std::filesystem::path copies_dir = "./copies";
    clean_directory(copies_dir);

    return setup;
}
