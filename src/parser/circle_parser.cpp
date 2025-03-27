#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "parser_helper.h"
#include "string_helper.h"
#include "math_helper.h"
#include "draw_helper.h"

#include "circle_parser.h"

std::vector<cv::Vec3f> detect_circles(cv::Mat img) {
    std::vector<cv::Vec3f> detected_circles;

    cv::HoughCircles(img, detected_circles, cv::HOUGH_GRADIENT, 1, img.rows / 8, 300, 50, 5, 50);

    return detected_circles;
}

std::optional<cv::Mat> circle_parser(cv::Mat img,
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

#ifdef DEBUG
    std::vector<cv::Point> box;

    for (const auto& point : corner_barcode.bounding_box) {
        box.push_back(cv::Point(point.x, point.y));
    }

    cv::polylines(debug_img, box, true, cv::Scalar(0, 255, 0), 2);
#endif

    meta = parse_metadata(corner_barcode.content);

    auto detected_circles = detect_circles(img);
    if (detected_circles.empty()) {
        printf("no circle found\n");
        return {};
    }

#ifdef DEBUG
    for (const auto& c : detected_circles) {
        cv::Point center(cvRound(c[0]), cvRound(c[1]));
        int radius = cvRound(c[2]);
        cv::circle(debug_img, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
        cv::circle(debug_img, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
    }
#endif

    std::vector<cv::Point2f> corner_points;
    std::vector<cv::Point2f> circle_pos;

    for (const auto& c : detected_circles) {
        circle_pos.push_back(cv::Point2f(c[0], c[1]));
    }

    auto mask = found_other_point(circle_pos, corner_points, corner_barcode);

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & mask)
            cv::circle(debug_img, corner_points[i], 3, cv::Scalar(0, 255, 255), -1);
    }
#endif

    auto mat = get_affine_transform(mask, dst_corner_points, corner_points);

    return mat;
}
