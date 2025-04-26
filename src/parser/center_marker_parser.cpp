/**
 * @file default_parser.cpp
 * @brief Default parser implementation. (do nothing)
 *
 */

#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "center_marker_parser.h"
#include "parser_helper.h"
#include "string_helper.h"
#include <draw_helper.h>

static cv::Point2f center_of_rec(cv::Rect rect) {
    return cv::Point2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

#define MIN_SIZE 25
#define MAX_SIZE 200

static bool discriminate(cv::Rect rect) {
    return rect.width > MIN_SIZE && rect.height > MIN_SIZE && rect.width < MAX_SIZE && rect.height < MAX_SIZE;
}

static std::vector<cv::Point2f> detect_shape(cv::Mat img) {
    std::vector<cv::Point2f> detected_shapes;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy; // unused; but could be used in drawContours
    cv::findContours(img, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        if (contour.size() < 5)
            continue;

        cv::Rect rect = cv::boundingRect(cv::Mat(contour));

        if (!discriminate(rect))
            continue;

        auto center = center_of_rec(rect);
        detected_shapes.push_back({ center.x, center.y });
    }

    return detected_shapes;
}

std::optional<DetectedBarcode> select_header_qrcode(const std::vector<DetectedBarcode>& barcodes) {
    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hztc")) {
            return barcode;
        }
    }
    return {};
}

static float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c) {
    cv::Point2f ab = b - a;
    cv::Point2f cb = b - c;

    float dot = ab.x * cb.x + ab.y * cb.y;
    float cross = ab.x * cb.y - ab.y * cb.x;

    return abs(abs(atan2(cross, dot)) - (M_PI / 2));
}

static float distance(cv::Point2f a, cv::Point2f b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

#define DISTANCE_THRESHOLD 200

std::vector<cv::Point2f> find_closest_point_corner(const std::vector<cv::Point2f>& detected_shapes,
                                                   const cv::Size& img_size, int& flag) {
    std::vector<cv::Point2f> corner_points(4, { 0, 0 });
    std::vector<cv::Point2f> candidate_top_left, candidate_top_right, candidate_bottom_left, candidate_bottom_right;
    for (const auto& shape : detected_shapes) {
        if (distance(shape, cv::Point2f(0, 0)) < DISTANCE_THRESHOLD) {
            candidate_top_left.push_back(shape);
        } else if (distance(shape, cv::Point2f(img_size.width, 0)) < DISTANCE_THRESHOLD) {
            candidate_top_right.push_back(shape);
        } else if (distance(shape, cv::Point2f(0, img_size.height)) < DISTANCE_THRESHOLD) {
            candidate_bottom_left.push_back(shape);
        } else if (distance(shape, cv::Point2f(img_size.width, img_size.height)) < DISTANCE_THRESHOLD) {
            candidate_bottom_right.push_back(shape);
        }
    }

    float scord = 10.f;
    for (const auto& tl : candidate_top_left) {
        for (const auto& tr : candidate_top_right) {
            for (const auto& br : candidate_bottom_right) {
                if (angle(tr, tl, br) < scord) {
                    corner_points[TOP_LEFT] = tl;
                    corner_points[TOP_RIGHT] = tr;
                    corner_points[BOTTOM_RIGHT] = br;
                    flag = TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_RIGHT_BF;
                    scord = angle(tl, tr, br);
                }
            }
            for (const auto& bl : candidate_bottom_left) {
                if (angle(tl, tr, bl) < scord) {
                    corner_points[TOP_LEFT] = tl;
                    corner_points[TOP_RIGHT] = tr;
                    corner_points[BOTTOM_LEFT] = bl;
                    flag = TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF;
                    scord = angle(tl, tr, bl);
                }
            }
        }
    }

    return corner_points;
}

std::optional<cv::Mat> center_marker_parser(const cv::Mat& img,
#ifdef DEBUG
                                            cv::Mat debug_img,
#endif
                                            Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {
    auto barcodes = identify_barcodes(img);

    if (barcodes.empty()) {
        printf("no barcode found\n");
        return {};
    }

#ifdef DEBUG
    draw_qrcode(barcodes, debug_img);
#endif

    auto corner_barcode_opt = select_header_qrcode(barcodes);

    if (!corner_barcode_opt) {
        printf("no header barcode found\n");
        return {};
    }

    auto corner_barcode = corner_barcode_opt.value();

    meta = parse_metadata(corner_barcode.content);

    auto detected_shapes = detect_shape(img);
    if (detected_shapes.empty()) {
        printf("no circle found\n");
        return {};
    }

#ifdef DEBUG
    for (const auto& c : detected_shapes) {
        cv::circle(debug_img, c, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
    }
#endif

    int flag = 0;

    auto corner_points = find_closest_point_corner(detected_shapes, img.size(), flag);

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & flag)
            cv::circle(debug_img, corner_points[i], 3, cv::Scalar(0, 255, 255), -1);
    }
#endif

    auto mat = get_affine_transform(flag, dst_corner_points, corner_points);
    return mat;
}