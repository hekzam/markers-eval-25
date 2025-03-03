#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>

#include <common.h>
#include <benchmark.hpp>
#include "utils/json_helper.h"
#include "utils/parser_helper.h"

#include "qrcode_parser.h"
#include "circle_parser.h"

std::unordered_map<std::string, Parser> parsers = {
    { "qrcode", { main_qrcode, draw_qrcode } },
    { "circle", { main_circle, draw_circle } },
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES DIR_IMAGE...\n");
        return 1;
    }

    // Création du répertoire de sortie
    std::filesystem::path output_dir{ argv[1] };
    std::filesystem::create_directories(output_dir);

    std::filesystem::path subimg_output_dir = output_dir.string() + std::string("/subimg");
    std::filesystem::create_directories(subimg_output_dir);

    // Lecture du fichier de description des AtomicBox
    std::ifstream atomic_boxes_file(argv[2]);
    if (!atomic_boxes_file.is_open()) {
        fprintf(stderr, "could not open file '%s'\n", argv[2]);
        return 1;
    }
    json atomic_boxes_json;
    try {
        atomic_boxes_json = json::parse(atomic_boxes_file);
    } catch (const json::exception& e) {
        fprintf(stderr, "could not json parse file '%s': %s", argv[2], e.what());
        return 1;
    }
    // printf("atomic_boxes: %s\n", atomic_boxes_json.dump(2).c_str());

    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    std::filesystem::path dir_path{ argv[3] };
    if (!std::filesystem::is_directory(dir_path)) {
        fprintf(stderr, "could not open directory '%s'\n", argv[3]);
        return 1;
    }

    /// TODO: add an argument to specify the parser
    auto parser = parsers["circle"];

    /// TODO: load page.json
    const cv::Point2f src_img_size{ 210, 297 }; // TODO: do not assume A4

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        cv::Mat img = cv::imread(entry.path(), cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        // TODO: use min and max for 90 ° rotate if needed
        // printf("dst_img_size: (%f, %f)\n", dst_img_size.x, dst_img_size.y);

        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

        Metadata meta;
        cv::Mat affine_transform;
        {
            BENCHMARK_BLOCK("parser");
            affine_transform = parser.parser(img, meta, dst_corner_points);
        }

        auto calibrated_img_col = redress_image(img, affine_transform);

        cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

        for (auto box : user_boxes_per_page[meta.page - 1]) {
            const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y + box->height },
                                                       cv::Point2f{ box->x, box->y + box->height } };
            std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);

            // Déssiner les contours de la boîte utilisateur
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 255), 2);
        }

        parser.draw_marker(calibrated_img_col, corner_markers, src_img_size, dimension);

        std::filesystem::path output_img_path_fname = entry.path().filename().replace_extension(".png");
        char* output_img_fname = nullptr;
        int nb = asprintf(&output_img_fname, "%s/cal-%s", output_dir.c_str(), output_img_path_fname.c_str());
        (void) nb;
        cv::imwrite(output_img_fname, calibrated_img_col);
        printf("output image: %s\n", output_img_fname);
        free(output_img_fname);
    }

    return 0;
}
