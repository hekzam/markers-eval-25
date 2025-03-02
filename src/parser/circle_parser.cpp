#include <vector>
#include <string>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "json_helper.h"
#include "parser_helper.h"
#include "string_helper.h"

#include "circle_parser.h"

std::vector<cv::Vec3f> detected_circles;

std::vector<cv::Vec3f> detect_circles(cv::Mat img) {
    std::vector<cv::Vec3f> detected_circles;

    cv::HoughCircles(img, detected_circles, cv::HOUGH_GRADIENT_ALT, 1, img.rows / 8, 300, 0.98f, 10, 50);

    return detected_circles;
}

float findArea(float a, float b, float c) {
    // Length of sides must be positive
    // and sum of any two sides
    // must be smaller than third side.
    if (a < 0 || b < 0 || c < 0 || (a + b <= c) || a + c <= b || b + c <= a) {
        throw std::invalid_argument("Invalid sides\n");
    }
    float s = (a + b + c) / 2;
    return sqrt(s * (s - a) * (s - b) * (s - c));
}

bool compare_area(const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
    return a.first < b.first;
}

int identify_corner(std::vector<cv::Vec3f>& detected_circles, std::vector<cv::Point2f>& corner_points,
                    DetectedBarcode& corner_barcode) {
    corner_points.resize(4);

    int found_mask = 0x00;

    cv::Mat mean_mat;
    cv::reduce(corner_barcode.bounding_box, mean_mat, 1, cv::REDUCE_AVG);
    corner_points[BOTTOM_RIGHT] = cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));
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

    std::vector<std::pair<float, cv::Point2f>> area_points;

    for (const auto& circle : detected_circles) {
        if (circle[0] == max_distance.second.x && circle[1] == max_distance.second.y)
            continue;

        float a =
            sqrt(pow(circle[0] - corner_points[BOTTOM_RIGHT].x, 2) + pow(circle[1] - corner_points[BOTTOM_RIGHT].y, 2));
        float b = sqrt(pow(circle[0] - corner_barcode.bounding_box[1].x, 2) +
                       pow(circle[1] - corner_barcode.bounding_box[1].y, 2));
        float c = sqrt(pow(circle[0] - max_distance.second.x, 2) + pow(circle[1] - max_distance.second.y, 2));

        float area = findArea(a, b, c);
        area_points.push_back({ area, cv::Point2f(circle[0], circle[1]) });
    }

    std::sort(area_points.begin(), area_points.end(), compare_area);

    corner_points[TOP_RIGHT] = area_points[0].second;
    found_mask |= (1 << TOP_RIGHT);

    corner_points[BOTTOM_LEFT] = area_points[1].second;
    found_mask |= (1 << BOTTOM_LEFT);

    if (corner_points[TOP_RIGHT].x < corner_points[BOTTOM_LEFT].x) {
        std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
    }

    return found_mask;
}

cv::Mat main_circle(cv::Mat img, Metadata& meta, std::vector<cv::Point2f> dst_corner_points) {

    auto barcodes = identify_barcodes(img);

    DetectedBarcode corner_barcode;
    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hzbr")) {
            corner_barcode = barcode;
            break;
        }
    }

    meta = parse_metadata(corner_barcode.content);

    detected_circles = detect_circles(img);

    std::vector<cv::Point2f> corner_points;
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

    for (auto box : corner_markers) {
        if (strncmp("marker barcode br", box->id.c_str(), 17) == 0)
            break;

        const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y + box->height },
                                                   cv::Point2f{ box->x, box->y + box->height } };
        std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
        cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
    }
}