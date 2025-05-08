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

static cv::Point2f center_of_rec(cv::Rect rect) {
    return cv::Point2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

#define MIN_SIZE 25
#define MAX_SIZE 200

static bool discriminate(cv::Rect rect) {
    return rect.width > MIN_SIZE && rect.height > MIN_SIZE && rect.width < MAX_SIZE && rect.height < MAX_SIZE;
}

static std::vector<std::pair<cv::Point2f, cv::Rect>> detect_shape(const cv::Mat& img, const cv::Point2i& offset) {
    std::vector<std::pair<cv::Point2f, cv::Rect>> detected_shapes;

    cv::Mat canny_img;
    cv::Canny(img, canny_img, 100, 100, 3);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy; // unused; but could be used in drawContours
    cv::findContours(canny_img, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, offset);

    for (const auto& contour : contours) {
        if (contour.size() < 5)
            continue;

        cv::Rect rect = cv::boundingRect(cv::Mat(contour));

        if (!discriminate(rect))
            continue;

        auto center = center_of_rec(rect);
        detected_shapes.push_back({ { center.x, center.y }, rect });
    }

    return detected_shapes;
}

std::optional<cv::Mat> shape_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {
    auto barcodes = identify_barcodes(img, (ZXing::BarcodeFormat) flag_barcode);

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

    // auto detected_shapes = detect_shape(img);
    auto detected_shapes = smaller_parse(img,
#ifdef DEBUG
                                         debug_img,
#endif
                                         detect_shape);
    if (detected_shapes.empty()) {
        printf("no circle found\n");
        return {};
    }

#ifdef DEBUG
    for (const auto& c : detected_shapes) {
        cv::rectangle(debug_img, c.second, cv::Scalar(0, 255, 0), 1);
        cv::circle(debug_img, c.first, 3, cv::Scalar(0, 255, 0), -1);
    }
#endif

    std::vector<cv::Point2f> shape_points;
    for (const auto& c : detected_shapes) {
        shape_points.push_back(c.first);
    }

    std::vector<cv::Point2f> corner_points;
    auto mask = found_other_point(shape_points, corner_points, center_of_box(corner_barcode.bounding_box));

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & mask)
            cv::circle(debug_img, corner_points[i], 10, cv::Scalar(0, 255, 255), -1);
    }
#endif

    auto mat = get_affine_transform(mask, dst_corner_points, corner_points);
    return mat;
}