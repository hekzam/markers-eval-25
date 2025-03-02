#include <vector>
#include <string>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "parser_helper.h"

std::vector<DetectedBarcode> identify_barcodes(cv::Mat img) {
    std::vector<DetectedBarcode> barcodes = {};

    if (img.type() != CV_8U)
        throw std::invalid_argument(
            "img has type != CV_8U while it should contain luminance information on 8-bit unsigned integers");

    if (img.cols < 2 || img.rows < 2)
        return {};

    auto iv =
        ZXing::ImageView(reinterpret_cast<const uint8_t*>(img.ptr()), img.cols, img.rows, ZXing::ImageFormat::Lum);
    auto z_barcodes = ZXing::ReadBarcodes(iv);

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

    return barcodes;
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

cv::Point center_of_box(std::vector<cv::Point2f> bounding_box) {
    cv::Mat mean_mat;
    cv::reduce(bounding_box, mean_mat, 1, cv::REDUCE_AVG);
    return cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));
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