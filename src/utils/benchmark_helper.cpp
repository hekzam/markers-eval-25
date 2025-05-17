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

void clean_directory_preserve_csv(const std::string& dir, const std::filesystem::path& csv_dir, CsvMode csv_mode) {
    if (std::filesystem::exists(dir)) {
        std::cout << "Cleaning directory: " << dir << std::endl;

        if (csv_mode == CsvMode::APPEND && std::filesystem::exists(csv_dir)) {
            std::filesystem::path dir_path(dir);
            std::filesystem::path temp_csv_dir = dir_path / "temp_csv_backup";
            std::filesystem::create_directories(temp_csv_dir);

            for (const auto& entry : std::filesystem::directory_iterator(csv_dir)) {
                if (entry.path().extension() == ".csv") {
                    std::filesystem::copy(entry.path(), temp_csv_dir / entry.path().filename());
                }
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (entry.path() != temp_csv_dir) {
                    std::filesystem::remove_all(entry.path());
                }
            }

            std::filesystem::create_directories(csv_dir);

            for (const auto& entry : std::filesystem::directory_iterator(temp_csv_dir)) {
                std::filesystem::copy(entry.path(), csv_dir / entry.path().filename(),
                                      std::filesystem::copy_options::overwrite_existing);
            }

            std::filesystem::remove_all(temp_csv_dir);
        } else {
            std::filesystem::remove_all(dir);
            std::filesystem::create_directories(dir);
        }
    } else {
        std::filesystem::create_directories(dir);
    }
}

BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column,
                                             bool create_subimg_dir, CsvMode csv_mode) {
    BenchmarkSetup setup;

    setup.output_dir = std::filesystem::path{ output_dir };
    setup.csv_output_dir = setup.output_dir / "csv";

    clean_directory_preserve_csv(setup.output_dir, setup.csv_output_dir, csv_mode);

    if (create_subimg_dir) {
        setup.subimg_output_dir = create_subdir(setup.output_dir, "subimg");
    }

    std::filesystem::create_directories(setup.csv_output_dir);

    std::filesystem::path copies_dir = "./copies";
    clean_directory(copies_dir);

    return setup;
}
