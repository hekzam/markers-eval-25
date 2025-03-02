#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "string_helper.h"
#include "qrcode_parser.h"
#include "circle_parser.h"

std::unordered_map<std::string, Parser> parsers = {
    { "qrcode", { main_qrcode, draw_qrcode } },
    { "circle", { main_circle, draw_circle } },
};

/**
 * @brief Différencie les AtomicBox en les classant par type et page.
 *
 * La fonction sépare les boîtes en trois catégories :
 * - Les marqueurs généraux (markers) dont l'identifiant commence par "marker barcode ".
 * - Les marqueurs de coin (corner_markers) pour lesquels l'identifiant contient les suffixes tl, tr, bl, br.
 * - Les boîtes utilisateur (user_boxes_per_page), regroupées par numéro de page.
 *
 * Un contrôle est effectué pour s'assurer que tous les marqueurs de coin sont présents. En cas d'absence,
 * une exception std::invalid_argument est levée.
 *
 * @param boxes Vecteur de pointeurs partagés sur les AtomicBox à différencier.
 * @param markers Vecteur de pointeurs partagés où seront stockés les marqueurs généraux.
 * @param corner_markers Vecteur de pointeurs partagés (de taille 4) où chaque index correspond à un coin :
 *        TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT.
 * @param user_boxes_per_page Vecteur de vecteurs de pointeurs partagés qui regroupe les boîtes utilisateur par page.
 *
 * @throw std::invalid_argument Si un ou plusieurs marqueurs de coin sont manquants dans la description JSON.
 */
void differentiate_atomic_boxes(std::vector<std::shared_ptr<AtomicBox>>& boxes,
                                std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page) {
    corner_markers.resize(4);
    user_boxes_per_page.clear();

    if (boxes.empty())
        return;
    int max_page = 1;

    for (const auto& box : boxes) {
        max_page = std::max(max_page, box->page);
    }
    user_boxes_per_page.resize(max_page);

    int corner_mask = 0;
    for (auto box : boxes) {
        if (starts_with(box->id, "hz")) {
            int corner = -1;
            if (starts_with(box->id, "hztl"))
                corner = TOP_LEFT;
            else if (starts_with(box->id, "hztr"))
                corner = TOP_RIGHT;
            else if (starts_with(box->id, "hzbl"))
                corner = BOTTOM_LEFT;
            else if (starts_with(box->id, "hzbr"))
                corner = BOTTOM_RIGHT;

            if (corner != -1) {
                corner_markers[corner] = box;
                corner_mask |= (1 << corner);
            }
        } else {
            user_boxes_per_page.at(box->page - 1).emplace_back(box);
        }
    }

    if (corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("some corner markers are missing in the atomic box JSON description");
}

/**
 * @brief Calcule le centre des marqueurs de coin.
 *
 * Pour chaque marqueur de coin fourni, la fonction calcule le centre de sa boîte englobante
 * en réduisant la matrice contenant les coordonnées, puis applique une transformation de redimensionnement
 * pour adapter ce centre aux dimensions de l'image destination.
 *
 * @param corner_markers Vecteur de pointeurs partagés sur les marqueurs de coin (AtomicBox) dans l'image source.
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image destination (largeur, hauteur).
 * @return std::vector<cv::Point2f> Vecteur contenant les points centraux des marqueurs redimensionnés.
 */
std::vector<cv::Point2f> calculate_center_of_marker(const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                                    const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size) {
    std::vector<cv::Point2f> corner_points;
    corner_points.resize(4);
    for (int corner = 0; corner < 4; ++corner) {
        auto marker = corner_markers[corner];
        const std::vector<cv::Point2f> marker_bounding_box = {
            cv::Point2f{ marker->x, marker->y }, cv::Point2f{ marker->x + marker->width, marker->y },
            cv::Point2f{ marker->x + marker->width, marker->y + marker->height },
            cv::Point2f{ marker->x, marker->y + marker->height }
        };

        // compute the center of the marker
        auto mean_point = center_of_box(marker_bounding_box);
        // printf("corner[%d] mean point: (%f, %f)\n", corner, mean_point.x, mean_point.y);

        corner_points[corner] = coord_scale(mean_point, src_img_size, dst_img_size);
    }
    return corner_points;
}

/**
 * @brief Redresse et calibre une image en fonction des marqueurs de coin.
 *
 * La fonction calcule les centres des marqueurs à partir des boîtes des coins, puis applique
 * une transformation affine (via la fonction main_qrcode) pour redresser l'image.
 * Ensuite, l'image redressée est convertie en couleur BGR.
 *
 * @param img Image en niveaux de gris à redresser.
 * @param meta Métadonnées associées à l'image.
 * @param corner_markers Vecteur de pointeurs partagés sur les marqueurs de coin (AtomicBox).
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image destination (largeur, hauteur).
 * @return cv::Mat Image redressée et convertie en couleur BGR.
 */
cv::Mat redress_image(cv::Mat img, cv::Mat affine_transform) {

    cv::Mat calibrated_img = img;
    warpAffine(img, calibrated_img, affine_transform, calibrated_img.size(), cv::INTER_LINEAR);

    cv::Mat calibrated_img_col;
    cv::cvtColor(calibrated_img, calibrated_img_col, cv::COLOR_GRAY2BGR);
    return calibrated_img_col;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES IMAGE...\n");
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

    /// TODO: add an argument to specify the parser
    auto parser = parsers["circle"];

    /// TODO: load page.json
    const cv::Point2f src_img_size{ 210, 297 }; // TODO: do not assume A4

    for (int i = 3; i < argc; ++i) {
        cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        // TODO: use min and max for 90 ° rotate if needed
        // printf("dst_img_size: (%f, %f)\n", dst_img_size.x, dst_img_size.y);

        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

        Metadata meta;
        auto affine_transform = parser.parser(img, meta, dst_corner_points);

        auto calibrated_img_col = redress_image(img, affine_transform);

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

            // Extraire la sous-image de la boîte utilisateur
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

            // Déssiner les contours de la boîte utilisateur
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(0, 0, 255), 2);
        }

        parser.draw_marker(calibrated_img_col, corner_markers, src_img_size, dimension);

        std::filesystem::path input_img_path{ argv[i] };
        std::filesystem::path output_img_path_fname = input_img_path.filename().replace_extension(".png");
        char* output_img_fname = nullptr;
        int nb = asprintf(&output_img_fname, "%s/cal-%s", output_dir.c_str(), output_img_path_fname.c_str());
        (void) nb;
        cv::imwrite(output_img_fname, calibrated_img_col);
        free(output_img_fname);

        /*cv::Mat with_markers;
        cv::cvtColor(img, with_markers, cv::COLOR_GRAY2BGR);
        std::string output_filename = std::string("/tmp/pout-") + std::to_string(i) + std::string(".png");
        cv::imwrite(output_filename, with_markers);*/
    }

    return 0;
}
