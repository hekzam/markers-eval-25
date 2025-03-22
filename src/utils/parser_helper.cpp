#include <vector>
#include <string>

#include <common.h>
#include "parser_helper.h"
#include "string_helper.h"
#include "math_helper.h"

#include "default_parser.h"
#include "qrcode_parser.h"
#include "circle_parser.h"
#include "custom_marker_parser.h"
#include "qrcode_empty_parser.h"
#include "aruco_parser.h"

std::unordered_map<std::string, Parser> parsers = {
    { "default", { default_parser } },
    { "qrcode", { main_qrcode } },
    { "circle", { main_circle } },
    { "aruco", { aruco_parser } },
    // { "custom", { custom_marker_parser, draw_custom_marker } }, drop custom parser because of his complexity
    { "empty", { main_qrcode_empty } },
};

std::vector<DetectedBarcode> identify_barcodes(cv::Mat img,
#ifdef ENABLE_ZBAR
                                               zbar_symbol_type_t flags
#else
                                               ZXing::BarcodeFormats flags
#endif
) {
    std::vector<DetectedBarcode> barcodes = {};

    if (img.type() != CV_8U)
        throw std::invalid_argument(
            "img has type != CV_8U while it should contain luminance information on 8-bit unsigned integers");

    if (img.cols < 2 || img.rows < 2)
        return {};

#ifdef ENABLE_ZBAR
    zbar::Image image(img.cols, img.rows, "Y800", (uchar*) img.data, img.cols * img.rows);
    zbar::ImageScanner scanner;

    // Configure scanner
    scanner.set_config(flags, zbar::ZBAR_CFG_ENABLE, 1);

    // Scan the image for barcodes and QRCodes
    int n = scanner.scan(image);

    // Print results
    for (zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        DetectedBarcode barcode;
        barcode.content = symbol->get_data();

        // Obtain location
        for (int i = 0; i < symbol->get_location_size(); i++) {
            barcode.bounding_box.emplace_back(cv::Point2f(symbol->get_location_x(i), symbol->get_location_y(i)));
        }

        barcodes.emplace_back(barcode);
    }
#else
    auto iv =
        ZXing::ImageView(reinterpret_cast<const uint8_t*>(img.ptr()), img.cols, img.rows, ZXing::ImageFormat::Lum);
    auto options = ZXing::ReaderOptions().setFormats(flags);
    auto z_barcodes = ZXing::ReadBarcodes(iv, options);
    for (const auto& b : z_barcodes) {
        DetectedBarcode barcode;
        barcode.content = b.text();

        std::vector<cv::Point> corners;
        for (int j = 0; j < 4; ++j) {
            const auto& p = b.position()[j];
            barcode.bounding_box.emplace_back(cv::Point2f(p.x, p.y));
        }
        barcodes.emplace_back(barcode);
    }
#endif

    return barcodes;
}

cv::Mat get_affine_transform(int found_corner_mask, const std::vector<cv::Point2f>& expected_corner_points,
                             const std::vector<cv::Point2f>& found_corner_points) {
    int nb_found = 0;
    std::vector<cv::Point2f> src, dst;
    src.reserve(3);
    dst.reserve(3);

    for (int corner = 0; corner < 4; ++corner) {
        if ((1 << corner) & found_corner_mask) {
            src.emplace_back(found_corner_points[corner]);
            dst.emplace_back(expected_corner_points[corner]);

            nb_found += 1;
            if (nb_found >= 3)
                break;
        }
    }

    if (nb_found != 3)
        throw std::invalid_argument("only " + std::to_string(nb_found) + " corners were found (3 or more required)");

    /*for (int i = 0; i < 3; ++i) {
    printf("src[%d]: (%f, %f)\n", i, src[i].x, src[i].y);
    printf("dst[%d]: (%f, %f)\n", i, dst[i].x, dst[i].y);
    }*/
    return cv::getAffineTransform(src, dst);
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

    cv::Mat calibrated_img = img.clone();
    warpAffine(img, calibrated_img, affine_transform, calibrated_img.size(), cv::INTER_LINEAR);

    cv::Mat calibrated_img_col;
    cv::cvtColor(calibrated_img, calibrated_img_col, cv::COLOR_GRAY2BGR);
    return calibrated_img_col;
}

float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c) {
    cv::Point2f ab = b - a;
    cv::Point2f cb = b - c;

    float dot = ab.x * cb.x + ab.y * cb.y;
    float cross = ab.x * cb.y - ab.y * cb.x;

    return atan2(cross, dot);
}

std::optional<cv::Mat> run_parser(const std::string& parser_name, cv::Mat img,
#ifdef DEBUG
                                  cv::Mat debug_img,
#endif
                                  Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {
    auto parser = parsers[parser_name];
    return parser.parser(img,
#ifdef DEBUG
                         debug_img,
#endif
                         meta, dst_corner_points);
}

std::optional<DetectedBarcode> select_bottom_right_corner(const std::vector<DetectedBarcode>& barcodes) {
    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hzbr")) {
            return barcode;
        }
    }
    return {};
}