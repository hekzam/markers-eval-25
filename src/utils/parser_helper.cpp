#include <vector>
#include <string>
#include <optional>

#include <common.h>
#include "parser_helper.h"
#include "string_helper.h"
#include "math_utils.h"

#include "default_parser.h"
#include "qrcode_parser.h"
#include "circle_parser.h"
#include "custom_marker_parser.h"
#include "qrcode_empty_parser.h"
#include "center_marker_parser.h"
#include "aruco_parser.h"
#include "shape_parser.h"
#include "cli_helper.h"

/// TODO: Corentin je te laisse le soin de t'en occuper stp
std::unordered_map<ParserType, Parser> parsers = {
    { ParserType::DEFAULT_PARSER, { default_parser } },
    { ParserType::QRCODE, { qrcode_parser } },
    { ParserType::CIRCLE, { circle_parser } },
    { ParserType::ARUCO, { aruco_parser } },
    { ParserType::SHAPE, { shape_parser } },
    { ParserType::CENTER_MARKER_PARSER, { center_marker_parser } },
    // { ParserType::CUSTOM_MARKER, { custom_marker_parser, draw_custom_marker } }, // drop custom parser because of his
    // complexity
    { ParserType::EMPTY, { qrcode_empty_parser } },
};

std::vector<DetectedBarcode> identify_barcodes(const cv::Mat& img,
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

std::optional<cv::Mat> get_affine_transform(int found_corner_mask,
                                            const std::vector<cv::Point2f>& expected_corner_points,
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

    if (nb_found != 3) {
        printf("not all corner points were found\n");
        return {};
    }

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
                                std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page) {
    corner_markers.resize(5);
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
            else if (starts_with(box->id, "hztc"))
                corner = TOP_CENTER;
            if (corner != -1) {
                corner_markers[corner] = box;
                corner_mask |= (1 << corner);
            }
        } else {
            user_boxes_per_page.at(box->page - 1).emplace_back(box);
        }
    }

    if (sum_mask(corner_mask) < 3)
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
std::vector<cv::Point2f>
calculate_center_of_marker(const std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                           const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size) {
    std::vector<cv::Point2f> corner_points;
    corner_points.resize(4);
    for (int corner = 0; corner < 4; ++corner) {
        if (corner_markers[corner].has_value() == false) {
            continue;
        }
        auto marker = corner_markers[corner].value();
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
 * une transformation affine (via la fonction qrcode_parser) pour redresser l'image.
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

int copy_config_to_flag(const CopyMarkerConfig& copy_marker_config) {
    int flag = 0;

    for (const auto& marker :
         { copy_marker_config.top_left, copy_marker_config.top_right, copy_marker_config.bottom_left,
           copy_marker_config.bottom_right, copy_marker_config.header }) {
        if (marker.type == MarkerType::QR_CODE)
            flag |= (int) ZXing::BarcodeFormat::QRCode;
        else if (marker.type == MarkerType::MICRO_QR_CODE)
            flag |= (int) ZXing::BarcodeFormat::MicroQRCode;
        else if (marker.type == MarkerType::DATAMATRIX)
            flag |= (int) ZXing::BarcodeFormat::DataMatrix;
        else if (marker.type == MarkerType::AZTEC)
            flag |= (int) ZXing::BarcodeFormat::Aztec;
        else if (marker.type == MarkerType::PDF417)
            flag |= (int) ZXing::BarcodeFormat::PDF417;
        else if (marker.type == MarkerType::RMQR)
            flag |= (int) ZXing::BarcodeFormat::RMQRCode;
        else if (marker.type == MarkerType::BARCODE)
            flag |= (int) ZXing::BarcodeFormat::Code128;
    }
    return flag;
}

std::string parser_type_to_string(ParserType parser_type) {
    switch (parser_type) {
        case ParserType::ARUCO:
            return "ARUCO";
        case ParserType::CIRCLE:
            return "CIRCLE";
        case ParserType::QRCODE:
            return "QRCODE";
        case ParserType::CUSTOM_MARKER:
            return "CUSTOM_MARKER";
        case ParserType::SHAPE:
            return "SHAPE";
        case ParserType::CENTER_MARKER_PARSER:
            return "CENTER_MARKER_PARSER";
        case ParserType::DEFAULT_PARSER:
            return "DEFAULT_PARSER";
        case ParserType::EMPTY:
            return "EMPTY";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Convertit une chaîne de caractères en type de parseur (ParserType)
 * 
 * @param parser_type_str Chaîne de caractères représentant le type de parseur
 * @return ParserType Le type de parseur correspondant, ou ParserType::QRCODE par défaut
 */
ParserType string_to_parser_type(const std::string& parser_type_str) {
    static const std::unordered_map<std::string, ParserType> parser_type_map = {
        {"ARUCO", ParserType::ARUCO},
        {"CIRCLE", ParserType::CIRCLE},
        {"QRCODE", ParserType::QRCODE},
        {"CUSTOM_MARKER", ParserType::CUSTOM_MARKER},
        {"SHAPE", ParserType::SHAPE},
        {"CENTER_MARKER_PARSER", ParserType::CENTER_MARKER_PARSER},
        {"DEFAULT_PARSER", ParserType::DEFAULT_PARSER},
        {"EMPTY", ParserType::EMPTY}
    };
    
    auto it = parser_type_map.find(parser_type_str);
    if (it != parser_type_map.end()) {
        return it->second;
    }
    
    return ParserType::QRCODE; // Valeur par défaut
}

std::optional<cv::Mat> run_parser(const ParserType& parser_type, cv::Mat img,
#ifdef DEBUG
                                  cv::Mat debug_img,
#endif
                                  Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {
    printf("run_parser: %s\n", parser_type_to_string(parser_type).c_str());
    auto parser = parsers[parser_type];
    return parser.parser(img,
#ifdef DEBUG
                         debug_img,
#endif
                         meta, dst_corner_points, flag_barcode);
}

std::optional<DetectedBarcode> select_bottom_right_corner(const std::vector<DetectedBarcode>& barcodes) {
    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hzbr")) {
            return barcode;
        }
    }
    return {};
}