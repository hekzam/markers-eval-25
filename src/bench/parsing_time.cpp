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

/**
 * @brief Dessine un contour autour d'une boîte
 *
 * @param box La boîte atomique à dessiner
 * @param image L'image sur laquelle dessiner
 * @param src_size Dimensions de l'image source
 * @param dst_size Dimensions de l'image de destination
 * @param color Couleur du contour
 * @param thickness Épaisseur de la ligne
 */
void draw_box_outline(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                      const cv::Point2f& dst_size, const cv::Scalar& color, int thickness = 2) {
    std::vector<cv::Point> raster_box = convert_to_raster(
        { cv::Point2f{ box->x, box->y }, cv::Point2f{ box->x + box->width, box->y },
          cv::Point2f{ box->x + box->width, box->y + box->height }, cv::Point2f{ box->x, box->y + box->height } },
        src_size, dst_size);
    cv::polylines(image, raster_box, true, color, thickness);
}

/**
 * @brief Dessine un cercle au centre d'une boîte
 *
 * @param box La boîte atomique
 * @param image L'image sur laquelle dessiner
 * @param src_size Dimensions de l'image source
 * @param dst_size Dimensions de l'image de destination
 * @param color Couleur du cercle
 * @param radius Rayon du cercle
 * @param thickness Épaisseur du cercle (-1 pour rempli)
 */
void draw_box_center(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                     const cv::Point2f& dst_size, const cv::Scalar& color, int radius = 3, int thickness = -1) {
    cv::Point center =
        convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } }, src_size, dst_size)[0];
    cv::circle(image, center, radius, color, thickness);
}

void parsing_benchmark(const std::unordered_map<std::string, Config>& config) {

    auto warmup_iterations = std::get<int>(config.at("warmup-iterations").value);
    auto nb_copies = std::get<int>(config.at("nb-copies").value);
    auto encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
    auto unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
    auto grey_level = std::get<int>(config.at("grey-level").value);
    auto dpi = std::get<int>(config.at("dpi").value);
    auto marker_config = std::get<std::string>(config.at("marker-config").value);

    CopyMarkerConfig copy_marker_config;
    if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
        std::cerr << "Invalid marker configuration: " << marker_config << std::endl;
        return;
    }
    ParserType selected_parser = select_parser_for_marker_config(copy_marker_config);

    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.unencoded_marker_size = unencoded_marker_size;
    style_params.grey_level = grey_level;
    style_params.dpi = dpi;

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", true, true);
    std::ofstream& benchmark_csv = benchmark_setup.benchmark_csv;

    generate_copies(nb_copies, warmup_iterations, style_params, copy_marker_config);

    json atomic_boxes_json = parse_json_file("./original_boxes.json");
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    std::filesystem::path dir_path{ "./copies" };
    if (!std::filesystem::is_directory(dir_path)) {
        throw std::runtime_error("could not open directory '" + dir_path.string() + "'");
    }

    std::vector<std::filesystem::directory_entry> all_entries;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        all_entries.push_back(entry);
    }

    if (warmup_iterations > 0) {
        std::cout << "Processing " << all_entries.size() << " files - first " << warmup_iterations
                  << " will be used as warm-up iterations" << std::endl;
    }

    int current_iteration = 0;
    const cv::Point2f src_img_size{ 210, 297 };

    for (const auto& entry : all_entries) {
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

        bool is_warmup = current_iteration < warmup_iterations;

        if (is_warmup) {
            std::cout << "Warmup iteration " << (current_iteration + 1) << "/" << warmup_iterations
                      << " with file: " << entry.path().filename().string() << std::endl;

            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);

            std::cout << "  Result: " << (affine_transform.has_value() ? "Success" : "Failed") << std::endl;
        } else {
            BenchmarkGuardSuccess benchmark_guard(entry.path().filename().string(), &benchmark_csv);
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            benchmark_guard.setSuccess(affine_transform.has_value());
        }

        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, benchmark_setup.output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not find the markers\n");
            current_iteration++;
            continue;
        }

        if (!is_warmup) {
            auto calibrated_img_col = redress_image(img, affine_transform.value());
            cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

            for (auto box : user_boxes_per_page[meta.page - 1]) {
                draw_box_outline(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 255));
            }

            for (auto box : corner_markers) {
                draw_box_outline(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 0));
                draw_box_center(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(0, 255, 0));
            }

            save_image(calibrated_img_col, benchmark_setup.output_dir, output_img_path_fname);
#ifdef DEBUG
            save_debug_img(debug_img, benchmark_setup.output_dir, output_img_path_fname);
#endif
        }

        current_iteration++;
    }
    std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and "
              << (all_entries.size() - warmup_iterations) << " measured iterations." << std::endl;
    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}