#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "default_parser.h"
#include "parser_helper.h"

std::optional<cv::Mat> default_parser(const cv::Mat& img,
#ifdef DEBUG
                                      cv::Mat debug_img,
#endif
                                      Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {
    return {};
}