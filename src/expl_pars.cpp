#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "utils/json_helper.h"
#include <memory>
#include "string_helper.h"
#include "qrcode_parser.h"

enum CornerBF { TOP_LEFT_BF = 0x01, TOP_RIGHT_BF = 0x02, BOTTOM_LEFT_BF = 0x04, BOTTOM_RIGHT_BF = 0x08 };

/**
 * @brief Analyse un objet JSON pour extraire les informations des AtomicBox et les stocker dans un vecteur.
 *
 * La fonction parcourt chaque élément du contenu JSON, crée une instance d'AtomicBox avec les attributs extraits,
 * et ajoute cette instance dans le vecteur passé par référence.
 *
 * @param content Objet JSON contenant les descriptions des AtomicBox. Chaque clé représente l'identifiant de la box.
 * @param boxes Vecteur dans lequel les AtomicBox analysées seront stockées.
 */
void parse_atomic_boxes(const json& content, std::vector<AtomicBox>& boxes) {
    for (const auto& [key, value] : content.items()) {
        AtomicBox box;
        box.id = key;
        box.page = value["page"];
        box.x = value["x"];
        box.y = value["y"];
        box.width = value["width"];
        box.height = value["height"];

        boxes.emplace_back(box);
    }
}

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
                                std::vector<std::shared_ptr<AtomicBox>>& markers,
                                std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page) {
    markers.clear();
    corner_markers.resize(4);
    user_boxes_per_page.clear();

    if (boxes.empty())
        return;
    int max_page = 1;

    for (const auto& box : boxes) {
        max_page = std::max(max_page, box->page);
    }
    user_boxes_per_page.resize(max_page);

    for (auto box : boxes) {
        if (starts_with(box->id, "marker barcode ")) {
            markers.emplace_back(box);
        } else {
            user_boxes_per_page.at(box->page - 1).emplace_back(box);
        }
    }

    int corner_mask = 0;
    for (auto box : markers) {
        int corner = -1;
        if (starts_with(box->id, "marker barcode tl"))
            corner = TOP_LEFT;
        else if (starts_with(box->id, "marker barcode tr"))
            corner = TOP_RIGHT;
        else if (starts_with(box->id, "marker barcode bl"))
            corner = BOTTOM_LEFT;
        else if (starts_with(box->id, "marker barcode br"))
            corner = BOTTOM_RIGHT;

        if (corner != -1) {
            corner_markers[corner] = box;
            corner_mask |= (1 << corner);
        }
    }

    if (corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("some corner markers are missing in the atomic box JSON description");
}

/**
 * @brief Redimensionne une coordonnée d'une image source vers une image destination.
 *
 * Cette fonction prend une coordonnée dans l'image source et la redimensionne proportionnellement
 * à l'image de destination.
 *
 * @param src_coord Coordonnée à redimensionner.
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image destination (largeur, hauteur).
 * @return cv::Point2f Coordonnée redimensionnée dans l'image destination.
 */
cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size,
                        const cv::Point2f& dst_img_size) {
    return cv::Point2f{
        (src_coord.x / src_img_size.x) * dst_img_size.x,
        (src_coord.y / src_img_size.y) * dst_img_size.y,
    };
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
        cv::Mat mean_mat;
        cv::reduce(marker_bounding_box, mean_mat, 1, cv::REDUCE_AVG);
        cv::Point2f mean_point{ mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1) };
        // printf("corner[%d] mean point: (%f, %f)\n", corner, mean_point.x, mean_point.y);

        corner_points[corner] = coord_scale(mean_point, src_img_size, dst_img_size);
    }
    return corner_points;
}

/**
 * @brief Convertit un ensemble de points en coordonnées flottantes vers des coordonnées raster (entiers).
 *
 * La fonction redimensionne chaque point en fonction de la transformation entre l'image source et l'image destination,
 * puis convertit les coordonnées flottantes en coordonnées entières arrondies.
 *
 * @param vec_points Vecteur de points en coordonnées flottantes.
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image destination (largeur, hauteur).
 * @return std::vector<cv::Point> Vecteur de points en coordonnées raster (entiers).
 */
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size) {
    std::vector<cv::Point> raster_points;
    raster_points.reserve(vec_points.size());
    for (const auto& point : vec_points) {
        auto scaled_point = coord_scale(point, src_img_size, dst_img_size);
        raster_points.emplace_back(cv::Point(round(scaled_point.x), round(scaled_point.y)));
    }
    return raster_points;
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
cv::Mat redress_image(cv::Mat img, Metadata& meta, std::vector<std::shared_ptr<AtomicBox>> corner_markers,
                      const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size) {
    auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

    auto affine_transform = main_qrcode(img, meta, dst_corner_points);

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
    std::vector<std::shared_ptr<AtomicBox>> markers;
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, markers, corner_markers, user_boxes_per_page);

    const cv::Point2f src_img_size{ 210, 297 }; // TODO: do not assume A4

    for (int i = 3; i < argc; ++i) {
        cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        // TODO: use min and max for 90 ° rotate if needed
        // printf("dst_img_size: (%f, %f)\n", dst_img_size.x, dst_img_size.y);
        Metadata meta;
        auto calibrated_img_col = redress_image(img, meta, corner_markers, src_img_size, dst_img_size);

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

        for (auto box : corner_markers) {
            if (strncmp("marker barcode br", box->id.c_str(), 17) == 0)
                break;

            const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y },
                                                       cv::Point2f{ box->x + box->width, box->y + box->height },
                                                       cv::Point2f{ box->x, box->y + box->height } };
            std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
        }

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
