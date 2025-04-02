#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "benchmark.hpp"
#include "utils/math_utils.h"
#include "utils/draw_helper.h"

struct BenchmarkParams {
    std::string output_dir = "./output";
    std::string atomic_boxes_file = "./original_boxes.json";
    std::string input_dir = "./copies";
    std::string copies_str = "1";
    int encoded_marker_size = 15;
    int fiducial_marker_size = 10;
    int grey_level = 0;
    int marker_config = ARUCO_WITH_QR_BR;
};

std::unordered_map<std::string, Config> default_config_time_copy = {
    { "output-dir", { "Output directory", "The directory where the output images will be saved", "./output" } },
    { "atomic-boxes-file",
      { "Atomic boxes file", "The path to the JSON file containing the atomic boxes", "./original_boxes.json" } },
    { "input-dir", { "Input directory", "The directory containing the input images", "./copies" } },
    { "nb-copies", { "Number of copies", "The number of copies to generate", 1 } },
    { "encoded-marker_size", { "Encoded marker size", "The size of the encoded markers", 15 } },
    { "fiducial-marker_size", { "Fiducial marker size", "The size of the fiducial markers", 10 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "marker-config", { "Marker configuration", "The configuration of the markers", ARUCO_WITH_QR_BR } }
};

bool constraint(std::unordered_map<std::string, Config> config) {
    if (std::get<std::string>(config["output-dir"].value).empty() ||
        std::get<std::string>(config["atomic-boxes-file"].value).empty() ||
        std::get<std::string>(config["input-dir"].value).empty()) {
        std::cerr << "Output directory, atomic boxes file and input directory must not be empty" << std::endl;
        return false;
    }

    if (!std::filesystem::exists(std::get<std::string>(config["atomic-boxes-file"].value))) {
        std::cerr << "Atomic boxes file '" << std::get<std::string>(config["atomic-boxes-file"].value)
                  << "' does not exist" << std::endl;
        return false;
    }
    if (std::get<int>(config["nb-copies"].value) <= 0 || std::get<int>(config["nb-copies"].value) > 50) {
        std::cerr << "Number of copies must be between 1 and 50" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Exécute le benchmark en fonction des paramètres fournis
 *
 * @param params Structure contenant les paramètres de benchmark
 * @param selected_parser Le type de parseur sélectionné
 */
void run_benchmark(int argc, char* argv[]) {
    auto opt_config = get_config(argc, argv, default_config_time_copy);
    if (!opt_config.has_value()) {
        print_help_config(default_config_time_copy);
        return;
    }
    auto config = opt_config.value();
    if (argc == 1) {
        add_missing_config(config, default_config_time_copy);
    } else {
        for (const auto& [key, value] : default_config_time_copy) {
            if (config.find(key) == config.end()) {
                config[key] = value;
            }
        }
    }

    if (!constraint(config)) {
        print_help_config(default_config_time_copy);
        return;
    }

    int nb_copies = std::get<int>(config["nb-copies"].value);
    auto atomic_boxes_file = std::get<std::string>(config["atomic-boxes-file"].value);
    auto input_dir = std::get<std::string>(config["input-dir"].value);
    auto copies_str = std::to_string(std::get<int>(config["nb-copies"].value));
    auto encoded_marker_size = std::get<int>(config["encoded-marker_size"].value);
    auto fiducial_marker_size = std::get<int>(config["fiducial-marker_size"].value);
    auto grey_level = std::get<int>(config["grey-level"].value);
    auto marker_config = std::get<int>(config["marker-config"].value);
    std::string selected_parser = select_parser_for_marker_config(marker_config);

    generate_copies(nb_copies, static_cast<MarkerConfig>(marker_config), encoded_marker_size, fiducial_marker_size,
                    grey_level);

    // Création et nettoyage des répertoires de sortie
    std::filesystem::path output_dir{ std::get<std::string>(config["output-dir"].value) };
    if (std::filesystem::exists(output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(output_dir);
    }
    std::filesystem::create_directories(output_dir);
    std::filesystem::path subimg_output_dir = create_subdir(output_dir, "subimg");
    std::filesystem::path csv_output_dir = create_subdir(output_dir, "csv");

    std::filesystem::path benchmark_csv_path = csv_output_dir / "benchmark_results.csv";
    std::ofstream benchmark_csv(benchmark_csv_path);
    if (benchmark_csv.is_open()) {
        benchmark_csv << "File,Time(ms),Success" << std::endl;
    }

    // Lecture et parsing du JSON
    json atomic_boxes_json = parse_json_file(atomic_boxes_file);
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    std::filesystem::path dir_path{ input_dir };
    if (!std::filesystem::is_directory(dir_path)) {
        throw std::runtime_error("could not open directory '" + dir_path.string() + "'");
    }

    const cv::Point2f src_img_size{ 210, 297 };
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        cv::Mat img = cv::imread(entry.path(), cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

        Metadata meta = { 0, 1, "" };
#ifdef DEBUG
        cv::Mat debug_img;
        cv::cvtColor(img, debug_img, cv::COLOR_GRAY2BGR);
#endif
        std::filesystem::path output_img_path_fname = entry.path().filename().replace_extension(".png");
        std::optional<cv::Mat> affine_transform;
        {
            BenchmarkGuardCSV benchmark_guard(entry.path().filename().string(), &benchmark_csv);
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            benchmark_guard.setSuccess(affine_transform.has_value());
        }
        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not find the markers\n");
            continue;
        }

        auto calibrated_img_col = redress_image(img, affine_transform.value());
        cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

        /// TODO: Refactoriser cette partie
        // Annotation des boîtes utilisateur
        for (auto box : user_boxes_per_page[meta.page - 1]) {
            std::vector<cv::Point> raster_box =
                convert_to_raster({ cv::Point2f{ box->x, box->y }, cv::Point2f{ box->x + box->width, box->y },
                                    cv::Point2f{ box->x + box->width, box->y + box->height },
                                    cv::Point2f{ box->x, box->y + box->height } },
                                  src_img_size, dimension);
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 255), 2);
        }

        // Annotation des marqueurs de coin
        for (auto box : corner_markers) {
            std::vector<cv::Point> raster_box =
                convert_to_raster({ cv::Point2f{ box->x, box->y }, cv::Point2f{ box->x + box->width, box->y },
                                    cv::Point2f{ box->x + box->width, box->y + box->height },
                                    cv::Point2f{ box->x, box->y + box->height } },
                                  src_img_size, dimension);
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
            cv::circle(calibrated_img_col,
                       convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } },
                                         src_img_size, dimension)[0],
                       3, cv::Scalar(0, 255, 0), -1);
        }

        save_image(calibrated_img_col, output_dir, output_img_path_fname);
#ifdef DEBUG
        save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
    }
    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}