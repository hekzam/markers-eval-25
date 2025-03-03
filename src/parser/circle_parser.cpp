#include <vector>
#include <string>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "json_helper.h"
#include "parser_helper.h"
#include "string_helper.h"

#include "circle_parser.h"

std::vector<cv::Vec3f> detected_circles;
DetectedBarcode corner_barcode;
std::vector<cv::Point2f> corner_points;

std::vector<cv::Vec3f> detect_circles(cv::Mat img) {
    std::vector<cv::Vec3f> detected_circles;

    // cv::HoughCircles(img, detected_circles, cv::HOUGH_GRADIENT_ALT, 1, img.rows / 8, 300, 0.98f, 10, 50);
    cv::HoughCircles(img, detected_circles, cv::HOUGH_GRADIENT, 1, img.rows / 8, 300, 30, 10, 50);

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

float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c) {
    cv::Point2f ab = b - a;
    cv::Point2f cb = b - c;

    float dot = ab.x * cb.x + ab.y * cb.y;
    float cross = ab.x * cb.y - ab.y * cb.x;

    return atan2(cross, dot);
}

bool compare_area(const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
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

    corner_points[TOP_RIGHT] = right_corner_points[0].second;
    found_mask |= (1 << TOP_RIGHT);

    corner_points[BOTTOM_LEFT] = right_corner_points[1].second;
    found_mask |= (1 << BOTTOM_LEFT);

    if (corner_points[TOP_RIGHT].x < corner_points[BOTTOM_LEFT].x) {
        std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
    }

    return found_mask;
}

cv::Mat main_circle(cv::Mat img, Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {

    auto barcodes = identify_barcodes(img);

    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hzbr")) {
            corner_barcode = barcode;
            break;
        }
    }

    meta = parse_metadata(corner_barcode.content);

    detected_circles = detect_circles(img);

    auto mask = identify_corner(detected_circles, corner_points, corner_barcode);

    if (mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("not all corner barcodes were found");

    auto mat = get_affine_transform(mask, dst_corner_points, corner_points);

    return mat;
}

void draw_circle(cv::Mat& calibrated_img_col, const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                 const cv::Point2f& src_img_size, const cv::Point2f& dimension) {

    for (const auto& c : detected_circles) {
        cv::Point center(cvRound(c[0]), cvRound(c[1]));
        int radius = cvRound(c[2]);
        cv::circle(calibrated_img_col, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
        cv::circle(calibrated_img_col, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
    }

    std::vector<cv::Point> box;

    for (const auto& point : corner_barcode.bounding_box) {
        box.push_back(cv::Point(point.x, point.y));
    }

    cv::polylines(calibrated_img_col, box, true, cv::Scalar(0, 0, 255), 2);

    for (auto box : corner_markers) {
        const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y + box->height },
                                                   cv::Point2f{ box->x, box->y + box->height } };
        std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
        cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
        cv::circle(calibrated_img_col,
                   convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } }, src_img_size,
                                     dimension)[0],
                   3, cv::Scalar(0, 255, 0), -1);
    }

    for (int i = 0; i < 4; ++i) {
        cv::circle(calibrated_img_col, corner_points[i], 3, cv::Scalar(0, 255, 255), -1);
    }
}