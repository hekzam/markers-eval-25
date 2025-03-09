/**
 * @file default_parser.cpp
 * @brief Default parser implementation. (do nothing)
 *
 */

#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "default_parser.h"
#include "parser_helper.h"

cv::Mat default_parser(cv::Mat img, Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {
    meta.id = 0;
    meta.page = 1;
    meta.name = "";

    return cv::Mat::eye(2, 3, CV_32F);
}

void draw_default(cv::Mat& calibrated_img_col, const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                  const cv::Point2f& src_img_size, const cv::Point2f& dimension) {
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