#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "parser_helper.h"
#include "string_helper.h"
#include "math_helper.h"
#include "draw_helper.h"

#include "circle_parser.h"

cv::Point2f center_of_rec(cv::Rect rect) {
    return cv::Point2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

std::vector<cv::Vec3f> detect_circles(cv::Mat img) {
    std::vector<cv::Vec3f> detected_circles;

    cv::HoughCircles(img, detected_circles, cv::HOUGH_GRADIENT, 1, img.rows / 8, 300, 50, 10, 30);

    // std::vector<std::vector<cv::Point>> contours;
    // std::vector<cv::Vec4i> hierarchy; // unused; but could be used in drawContours
    // cv::findContours(img, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    // for (const auto& contour : contours) {
    //     if (contour.size() < 5)
    //         continue;

    //     cv::Rect rect = cv::boundingRect(cv::Mat(contour));

    //     auto center = center_of_rec(rect);
    //     detected_circles.push_back({ center.x, center.y, 10 });
    // }

    return detected_circles;
}

float findArea(cv::Point2f a, cv::Point2f b, cv::Point2f c) {
    float side_a = sqrt(pow(b.x - c.x, 2) + pow(b.y - c.y, 2));
    float side_b = sqrt(pow(a.x - c.x, 2) + pow(a.y - c.y, 2));
    float side_c = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));

    if (side_a < 0 || side_b < 0 || side_c < 0 || (side_a + side_b <= side_c) || side_a + side_c <= side_b ||
        side_b + side_c <= side_a) {
        throw std::invalid_argument("Invalid sides\n");
    }
    float s = (side_a + side_b + side_c) / 2;
    return sqrt(s * (s - side_a) * (s - side_b) * (s - side_c));
}

static bool compare_area(const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
    return a.first < b.first;
}

int identify_corner(std::vector<cv::Vec3f>& detected_circles, std::vector<cv::Point2f>& corner_points,
                    DetectedBarcode& corner_barcode) {
    corner_points.resize(4);

    int found_mask = 0x00;

    corner_points[BOTTOM_RIGHT] = center_of_box(corner_barcode.bounding_box);
    found_mask |= (1 << BOTTOM_RIGHT);

    std::pair<float, cv::Point2f> max_distance = { 0, cv::Point2f(0, 0) };

    for (const auto& c : detected_circles) {
        float x = c[0] - corner_points[BOTTOM_RIGHT].x;
        float y = c[1] - corner_points[BOTTOM_RIGHT].y;
        float distance = sqrt(pow(x, 2) + pow(y, 2));
        if (distance > max_distance.first) {
            max_distance = { distance, cv::Point2f(c[0], c[1]) };
        }
    }

    corner_points[TOP_LEFT] = max_distance.second;
    found_mask |= (1 << TOP_LEFT);

    std::vector<std::pair<float, cv::Point2f>> right_corner_points;

    for (const auto& circle : detected_circles) {
        if (circle[0] == max_distance.second.x && circle[1] == max_distance.second.y)
            continue;

        float angle_corner =
            angle(corner_points[TOP_LEFT], cv::Point2f(circle[0], circle[1]), corner_points[BOTTOM_RIGHT]);
        angle_corner = angle_corner < 0 ? angle_corner + M_PI : angle_corner;
        angle_corner = abs(angle_corner - M_PI / 2);
        right_corner_points.push_back({ angle_corner, cv::Point2f(circle[0], circle[1]) });
    }

    std::sort(right_corner_points.begin(), right_corner_points.end(), compare_area);

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

std::optional<cv::Mat> main_circle(cv::Mat img,
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
    auto mask = identify_corner(detected_circles, corner_points, corner_barcode);

#ifdef DEBUG
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & mask)
            cv::circle(debug_img, corner_points[i], 3, cv::Scalar(0, 255, 255), -1);
    }
#endif

    if (mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        return {};

    auto mat = get_affine_transform(mask, dst_corner_points, corner_points);

    return mat;
}
