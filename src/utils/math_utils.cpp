#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <type_traits> 
#include <cmath>  

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

int found_other_point(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_points,
                      cv::Point2f corner_barcode) {
    corner_points.resize(4);

    int found_mask = 0x00;

    corner_points[BOTTOM_RIGHT] = corner_barcode;
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

    std::vector<std::tuple<float, cv::Point2f, float>> right_corner_points;

    for (const auto& point : points) {
        if (point == max_distance.second)
            continue;

        float angle_corner = angle(corner_points[TOP_LEFT], point, corner_points[BOTTOM_RIGHT]);
        float right_angle = abs(abs(angle_corner) - (M_PI / 2));
        right_corner_points.push_back({ right_angle, point, angle_corner });
    }

    std::sort(right_corner_points.begin(), right_corner_points.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    bool find_a_corner = false;

    if (right_corner_points.size() > 0 && std::get<0>(right_corner_points[0]) < 0.1f) {
        corner_points[TOP_RIGHT] = std::get<1>(right_corner_points[0]);
        found_mask |= (1 << TOP_RIGHT);
        find_a_corner = true;
    }

    if (right_corner_points.size() > 1 && std::get<0>(right_corner_points[1]) < 0.1f) {
        corner_points[BOTTOM_LEFT] = std::get<1>(right_corner_points[1]);
        found_mask |= (1 << BOTTOM_LEFT);
        find_a_corner = true;
    }

    if (right_corner_points.size() > 0 && std::get<2>(right_corner_points[0]) > 0) {
        std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
    }

    if (!find_a_corner) {
        std::vector<std::tuple<cv::Point2f, cv::Point2f, float, float>> right_corner_points;
        corner_points[TOP_LEFT] = { 0, 0 };
        found_mask &= ~(1 << TOP_LEFT);
        for (int i = 0; i < points.size(); ++i) {
            for (int j = i + 1; j < points.size(); ++j) {
                float angle_corner = angle(points[i], corner_points[BOTTOM_RIGHT], points[j]);
                float right_angle = abs(abs(angle_corner) - (M_PI / 2));
                right_corner_points.push_back({ points[i], points[j], right_angle, angle_corner });
            }
        }

        std::sort(right_corner_points.begin(), right_corner_points.end(),
                  [](const auto& a, const auto& b) { return std::get<2>(a) < std::get<2>(b); });

        if (right_corner_points.size() > 0 && std::get<2>(right_corner_points[0]) < 0.1f) {
            corner_points[TOP_RIGHT] = std::get<0>(right_corner_points[0]);
            corner_points[BOTTOM_LEFT] = std::get<1>(right_corner_points[0]);
            found_mask |= (1 << TOP_RIGHT);
            found_mask |= (1 << BOTTOM_LEFT);
            if (std::get<3>(right_corner_points[0]) > 0) {
                std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
            }
        }
    }

    return found_mask;
}

int sum_mask(int mask) {
    int sum = 0;
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & mask) {
            sum += 1;
        }
    }
    return sum;
}

cv::Mat translate(float x, float y) {
    cv::Mat out = cv::Mat::eye(3, 3, CV_32F);
    out.at<float>(0, 2) += x;
    out.at<float>(1, 2) += y;
    return out;
}

cv::Mat rotate(float angle) {
    cv::Mat out = cv::Mat::eye(3, 3, CV_32F);
    float rad = angle * M_PI / 180.0;
    out.at<float>(0, 0) = cos(rad);
    out.at<float>(0, 1) = -sin(rad);
    out.at<float>(1, 0) = sin(rad);
    out.at<float>(1, 1) = cos(rad);
    return out;
}

cv::Mat rotate_center(float angle, float cx, float cy) {
    cv::Mat out = cv::Mat::eye(3, 3, CV_32F);
    float rad = angle * M_PI / 180.0;
    out.at<float>(0, 0) = cos(rad);
    out.at<float>(0, 1) = -sin(rad);
    out.at<float>(1, 0) = sin(rad);
    out.at<float>(1, 1) = cos(rad);
    out.at<float>(0, 2) = cx * (1 - cos(rad)) + cy * sin(rad);
    out.at<float>(1, 2) = cy * (1 - cos(rad)) - cx * sin(rad);
    return out;
}

void print_mat(cv::Mat mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            std::cout << mat.at<float>(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

double percentage_to_offset(int depth, double percentage) {
    double max_value;

    if (depth == CV_8U) {
        max_value = 255.0;
    } else if (depth == CV_16U) {
        max_value = 65535.0;
    } else if (depth == CV_32F) {
        max_value = 1.0; // Si normalisé entre 0 et 1
    } else {
        CV_Error(cv::Error::StsBadArg, "Unsupported image type");
    }

    return (percentage / 100.0) * max_value;
}

double percentage_to_dispersion(int depth, double percentage) {
    double max_value;

    if (depth == CV_8U) {
        max_value = 255.0;
    } else if (depth == CV_16U) {
        max_value = 65535.0;
    } else if (depth == CV_32F) {
        max_value = 1.0; // Si l'image est normalisée entre 0 et 1
    } else {
        CV_Error(cv::Error::StsBadArg, "Type d'image non supporté");
    }

    return (percentage / 100.0) * max_value;
}

