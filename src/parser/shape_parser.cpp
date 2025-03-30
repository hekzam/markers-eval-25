/**
 * @file default_parser.cpp
 * @brief Default parser implementation. (do nothing)
 *
 */

#include <vector>
#include <string>

#include <common.h>

#include "json_helper.h"
#include "parser_helper.h"
#include "math_utils.h"
#include "draw_helper.h"

#include "shape_parser.h"

cv::Point2f center_of_rec(cv::Rect rect) {
    return cv::Point2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

#define MIN_SIZE 25
#define MAX_SIZE 100

bool discriminate(cv::Rect rect) {
    return rect.width > MIN_SIZE && rect.height > MIN_SIZE && rect.width < MAX_SIZE && rect.height < MAX_SIZE;
}

std::vector<cv::Point2f> detect_shape(cv::Mat img) {
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

std::optional<cv::Mat> shape_parser(cv::Mat img,
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

    auto corner_barcode_opt = select_bottom_right_corner(barcodes);

    if (!corner_barcode_opt) {
        printf("no corner barcode found\n");
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

    std::vector<cv::Point2f> corner_points;
    auto mask = found_other_point(detected_shapes, corner_points, corner_barcode);

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & mask)
            cv::circle(debug_img, corner_points[i], 3, cv::Scalar(0, 255, 255), -1);
    }
#endif

    auto mat = get_affine_transform(mask, dst_corner_points, corner_points);
    return mat;
}