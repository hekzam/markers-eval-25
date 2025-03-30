#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>
#include <iomanip>
#include <sstream>

#include <common.h>
#include <benchmark.hpp>
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/math_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/draw_helper.h"
#include "external-tools/create_copy.h"
#include "command-line-interface/command_line_interface.h"

/**
 * @brief Structure pour stocker les paramètres de benchmark
 */
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

/**
 * @brief Sélectionne le type de parseur approprié en fonction de la configuration de marqueur choisie
 *
 * @param marker_config Le numéro de configuration de marqueur (1-10)
 * @return std::string Le type de parseur à utiliser
 */
std::string select_parser_for_marker_config(int marker_config) {
    switch (marker_config) {
        case QR_ALL_CORNERS:
        case QR_BOTTOM_RIGHT_ONLY:
            return to_string(ParserType::QRCODE);
        case CIRCLES_WITH_QR_BR:
        case TOP_CIRCLES_QR_BR:
        case CIRCLE_OUTLINES_WITH_QR_BR:
            return to_string(ParserType::CIRCLE);
        case CUSTOM_SVG_WITH_QR_BR:
            return to_string(ParserType::CUSTOM_MARKER);
        case ARUCO_WITH_QR_BR:
        case TWO_ARUCO_WITH_QR_BR:
            return to_string(ParserType::ARUCO);
        case SQUARES_WITH_QR_BR:
        case SQUARE_OUTLINES_WITH_QR_BR:
        default:
            return to_string(ParserType::DEFAULT);
    }
}

/**
 * @brief Affiche la configuration actuelle du benchmark
 *
 * @param params Structure contenant les paramètres de benchmark
 */
void display_configuration(const BenchmarkParams& params) {
    std::vector<std::pair<std::string, std::string>> config_pairs = {
        { "Output directory", params.output_dir },
        { "Atomic boxes file", params.atomic_boxes_file },
        { "Input directory", params.input_dir },
        { "Copies", params.copies_str },
        { "Encoded marker size", std::to_string(params.encoded_marker_size) },
        { "Fiducial marker size", std::to_string(params.fiducial_marker_size) },
        { "Grey level", std::to_string(params.grey_level) },
        { "Marker config", std::to_string(params.marker_config) }
    };

    display_configuration_recap("Configuration:", config_pairs);
}

/**
 * @brief Récupère les paramètres de benchmark à partir de l'entrée utilisateur
 *
 * @return BenchmarkParams Structure contenant les paramètres de benchmark
 */
BenchmarkParams get_benchmark_params() {
    BenchmarkParams params;
    params.output_dir = get_user_input("Output directory", params.output_dir);
    params.atomic_boxes_file = get_user_input("Atomic boxes JSON file path", params.atomic_boxes_file);
    params.input_dir = get_user_input("Input directory", params.input_dir);
    params.copies_str = get_user_input("Number of copies", params.copies_str);

    const int min_marker_size = 5, max_marker_size = 50;
    params.encoded_marker_size =
        get_user_input("Encoded marker size", params.encoded_marker_size, &min_marker_size, &max_marker_size);
    params.fiducial_marker_size =
        get_user_input("Fiducial marker size", params.fiducial_marker_size, &min_marker_size, &max_marker_size);

    const int min_grey = 0, max_grey = 255;
    params.grey_level = get_user_input("Grey level", params.grey_level, &min_grey, &max_grey);

    int marker_config_default =
        display_marker_configs(marker_configs, ARUCO_WITH_QR_BR, "Available marker configurations:");
    const int min_config = 1, max_config = 10;
    params.marker_config =
        get_user_input("Marker configuration (1-10)", marker_config_default, &min_config, &max_config);

    return params;
}

/**
 * @brief Exécute le benchmark en fonction des paramètres fournis
 *
 * @param params Structure contenant les paramètres de benchmark
 * @param selected_parser Le type de parseur sélectionné
 */
void run_benchmark(const BenchmarkParams& params, const std::string& selected_parser) {
    int nb_copies = std::stoi(params.copies_str);
    if (nb_copies <= 0 || nb_copies > 50) {
        throw std::runtime_error("Number of copies must be between 1 and 50");
    }

    generate_copies(nb_copies, static_cast<MarkerConfig>(params.marker_config), params.encoded_marker_size,
                    params.fiducial_marker_size, params.grey_level);

    // Création et nettoyage des répertoires de sortie
    std::filesystem::path output_dir{ params.output_dir };
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

    if (!std::filesystem::exists(params.atomic_boxes_file)) {
        throw std::runtime_error("Atomic boxes file '" + params.atomic_boxes_file + "' does not exist");
    }

    // Lecture et parsing du JSON
    json atomic_boxes_json = parse_json_file(params.atomic_boxes_file);
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    std::filesystem::path dir_path{ params.input_dir };
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

int main(int argc, char* argv[]) {
    display_banner();
    BenchmarkParams params = get_benchmark_params();
    std::string selected_parser = select_parser_for_marker_config(params.marker_config);
    display_configuration(params);

    if (params.output_dir.empty() || params.atomic_boxes_file.empty() || params.input_dir.empty() ||
        params.copies_str.empty()) {
        throw std::runtime_error("Required arguments cannot be empty.");
    }

    run_benchmark(params, selected_parser);
    return 0;
}
/// TODO: load page.json