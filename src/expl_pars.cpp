/**
 * @file expl_pars.cpp
 * @brief Outil d'analyse d'images et d'extraction de zones marquées
 *
 * Ce programme permet d'analyser des images contenant des marqueurs spécifiques,
 * de les redresser selon une transformation affine, et d'extraire des zones d'intérêt
 * correspondant à des "boîtes atomiques" définies dans un fichier JSON.
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>

#include <common.h>
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/string_helper.h"
#include "utils/math_utils.h"
#include "utils/draw_helper.h"

/**
 * @brief Point d'entrée du programme d'analyse d'images
 *
 * Ce programme analyse des images contenant des marqueurs, extrait des zones
 * spécifiées dans un fichier de description JSON, et sauvegarde:
 * - Les images redressées avec les zones d'intérêt surlignées
 * - Les sous-images correspondant à chaque zone d'intérêt
 *
 * @param argc Nombre d'arguments passés au programme
 * @param argv Tableau des arguments:
 *        - argv[1]: Répertoire de sortie
 *        - argv[2]: Fichier JSON de description des zones à extraire (AtomicBox)
 *        - argv[3...]: Images à analyser
 * @return int Code de retour (0 en cas de succès, 1 en cas d'erreur)
 */
int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES IMAGE...\n");
        return 1;
    }

    std::filesystem::path output_dir{ argv[1] };
    std::filesystem::create_directories(output_dir);

    std::filesystem::path subimg_output_dir = output_dir.string() + std::string("/subimg");
    std::filesystem::create_directories(subimg_output_dir);

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
    std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    /// TODO: load page.json
    const cv::Point2f src_img_size{ 210, 297 }; // TODO: do not assume A4

    for (int i = 3; i < argc; ++i) {
        cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        /// TODO: use min and max for 90 ° rotate if needed
        // printf("dst_img_size: (%f, %f)\n", dst_img_size.x, dst_img_size.y);

        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

#ifdef DEBUG
        cv::Mat debug_img;
        cv::cvtColor(img, debug_img, cv::COLOR_GRAY2BGR);
#endif

        std::filesystem::path input_img_path{ argv[i] };
        std::filesystem::path output_img_path_fname = input_img_path.filename().replace_extension(".png");

        Metadata meta;
        auto affine_transform = run_parser(ParserType::SHAPE, img,
#ifdef DEBUG
                                           debug_img,
#endif
                                           meta, dst_corner_points);

        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not parse image '%s'\n", argv[i]);
            return 1;
        }

        auto calibrated_img_col = redress_image(img, affine_transform.value());

        cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

        for (auto box : user_boxes_per_page[meta.page - 1]) {
            const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y + box->height },
                                                       cv::Point2f{ box->x, box->y + box->height } };

            std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);

            int min_x = INT_MAX;
            int min_y = INT_MAX;
            int max_x = INT_MIN;
            int max_y = INT_MIN;
            for (auto v : raster_box) {
                min_x = std::min(min_x, v.x);
                min_y = std::min(min_y, v.y);
                max_x = std::max(max_x, v.x);
                max_y = std::max(max_y, v.y);
            }

            cv::Range rows(min_y, max_y);
            cv::Range cols(min_x, max_x);
            // printf("%d,%s: (%d,%d) -> (%d,%d)\n", copy, box->id.c_str(), min_x, min_y, max_x, max_y);
            cv::Mat subimg = calibrated_img_col(rows, cols);

            char* output_img_fname = nullptr;
            int nb =
                asprintf(&output_img_fname, "%s/subimg/raw-%d-%s.png", output_dir.c_str(), meta.id, box->id.c_str());
            (void) nb;
            printf("box fname: %s\n", output_img_fname);
            cv::imwrite(output_img_fname, subimg);
            free(output_img_fname);

            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 255), 2);
        }

        for (auto box : corner_markers) {
            if (box.has_value() == false) {
                continue;
            }
            auto marker = box.value();
            const std::vector<cv::Point2f> vec_box = {
                cv::Point2f{ marker->x, marker->y }, cv::Point2f{ marker->x + marker->width, marker->y },
                cv::Point2f{ marker->x + marker->width, marker->y + marker->height },
                cv::Point2f{ marker->x, marker->y + marker->height }
            };
            std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);

            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);

            cv::circle(
                calibrated_img_col,
                convert_to_raster({ cv::Point2f{ marker->x + marker->width / 2, marker->y + marker->height / 2 } },
                                  src_img_size, dimension)[0],
                3, cv::Scalar(0, 255, 0), -1);
        }

        char* output_img_fname = nullptr;
        int nb = asprintf(&output_img_fname, "%s/cal-%s", output_dir.c_str(), output_img_path_fname.c_str());
        (void) nb;
        cv::imwrite(output_img_fname, calibrated_img_col);
        printf("output image: %s\n", output_img_fname);
        free(output_img_fname);

#ifdef DEBUG
        save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
    }

    return 0;
}
