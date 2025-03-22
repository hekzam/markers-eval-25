#include <vector>

#include <common.h>

#include "parser_helper.h"

void draw_qrcode(std::vector<DetectedBarcode>& barcodes, cv::Mat& debug_img) {
    for (const auto& barcode : barcodes) {
        std::vector<cv::Point> box;

        for (const auto& point : barcode.bounding_box) {
            box.push_back(cv::Point(point.x, point.y));
        }

        cv::polylines(debug_img, box, true, cv::Scalar(0, 0, 255), 2);
    }
}