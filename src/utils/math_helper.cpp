#include <vector>

#include <common.h>
#include "parser_helper.h"

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

cv::Point2f center_of_box(std::vector<cv::Point2f> bounding_box) {
    cv::Mat mean_mat;
    cv::reduce(bounding_box, mean_mat, 1, cv::REDUCE_AVG);
    return cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));
}

float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c) {
    cv::Point2f ab = b - a;
    cv::Point2f cb = b - c;

    float dot = ab.x * cb.x + ab.y * cb.y;
    float cross = ab.x * cb.y - ab.y * cb.x;

    return atan2(cross, dot);
}

static bool compare(const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
    return a.first < b.first;
}

int found_other_point(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_points,
                      DetectedBarcode& corner_barcode) {
    corner_points.resize(4);

    int found_mask = 0x00;

    corner_points[BOTTOM_RIGHT] = center_of_box(corner_barcode.bounding_box);
    found_mask |= (1 << BOTTOM_RIGHT);

    std::pair<float, cv::Point2f> max_distance = { 0, cv::Point2f(0, 0) };

    for (const auto& point : points) {
        float x = point.x - corner_points[BOTTOM_RIGHT].x;
        float y = point.y - corner_points[BOTTOM_RIGHT].y;
        float distance = sqrt(pow(x, 2) + pow(y, 2));
        if (distance > max_distance.first) {
            max_distance = { distance, point };
        }
    }

    corner_points[TOP_LEFT] = max_distance.second;
    found_mask |= (1 << TOP_LEFT);

    std::vector<std::pair<float, cv::Point2f>> right_corner_points;

    for (const auto& point : points) {
        if (point == max_distance.second)
            continue;

        float angle_corner = angle(corner_points[TOP_LEFT], point, corner_points[BOTTOM_RIGHT]);
        angle_corner = angle_corner < 0 ? angle_corner + M_PI : angle_corner;
        angle_corner = abs(angle_corner - M_PI / 2);
        right_corner_points.push_back({ angle_corner, point });
    }

    std::sort(right_corner_points.begin(), right_corner_points.end(), compare);

    if (right_corner_points.size() < 2) {
        return found_mask;
    }

    corner_points[TOP_RIGHT] = right_corner_points[0].second;
    found_mask |= (1 << TOP_RIGHT);

    corner_points[BOTTOM_LEFT] = right_corner_points[1].second;
    found_mask |= (1 << BOTTOM_LEFT);

    if (corner_points[TOP_RIGHT].x < corner_points[BOTTOM_LEFT].x) {
        std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
    }

    return found_mask;
}