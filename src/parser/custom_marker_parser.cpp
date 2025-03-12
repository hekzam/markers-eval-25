/**
 * @file custom_marker_parser.cpp
 * @brief Custom marker parser implementation. (not finish because way to hard to detect shape of marker)
 */

#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "custom_marker_parser.h"
#include "parser_helper.h"

bool is_between(int x, int a, int b) {
    return a <= x && x <= b;
}

std::vector<cv::Point> squared_contour(const std::vector<cv::Point>& contour) {

    int x_min = INT_MAX;
    int x_max = INT_MIN;
    int y_min = INT_MAX;
    int y_max = INT_MIN;

    for (const auto& point : contour) {
        x_min = std::min(x_min, point.x);
        x_max = std::max(x_max, point.x);
        y_min = std::min(y_min, point.y);
        y_max = std::max(y_max, point.y);
    }

    std::vector<cv::Point> squared_contour = {
        cv::Point(x_min, y_min),
        cv::Point(x_max, y_min),
        cv::Point(x_max, y_max),
        cv::Point(x_min, y_max),
    };

    return squared_contour;
}

std::vector<std::vector<cv::Point>> detect_marker(cv::Mat img_gray) {
    cv::Mat thresh;
    cv::threshold(img_gray, thresh, 150, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> filtered_contours;
    for (int i = 0; i < contours.size(); i++) {
        if (is_between(contours[i].size(), 15, 30)) {
            filtered_contours.push_back((contours[i]));
        }
    }

    return filtered_contours;
}

std::optional<cv::Mat> custom_marker_parser(cv::Mat img, Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {
    meta.id = 0;
    meta.page = 1;
    meta.name = "";

    return cv::Mat::eye(2, 3, CV_32F);
}

void draw_custom_marker(cv::Mat& calibrated_img_col, const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                        const cv::Point2f& src_img_size, const cv::Point2f& dimension) {

    cv::Mat img_gray;
    cv::cvtColor(calibrated_img_col, img_gray, cv::COLOR_BGR2GRAY);
    auto contours = detect_marker(img_gray);

    for (auto box : corner_markers) {
        const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y },
                                                   cv::Point2f{ box->x + box->width, box->y + box->height },
                                                   cv::Point2f{ box->x, box->y + box->height } };
        std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
        cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
    }

    cv::drawContours(calibrated_img_col, contours, -1, cv::Scalar(0, 255, 0), 2);
}