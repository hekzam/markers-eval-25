/**
 * @file default_parser.cpp
 * @brief Default parser implementation. (do nothing)
 *
 */

#include <vector>
#include <string>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include "json_helper.h"
#include "default_parser.h"

cv::Mat default_parser(cv::Mat img, Metadata& meta, std::vector<cv::Point2f> dst_corner_points) {
    meta.id = 0;
    meta.page = 1;
    meta.name = "";

    return cv::Mat::eye(2, 3, CV_32F);
}