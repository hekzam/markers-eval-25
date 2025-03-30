#include <vector>
#include <common.h>
#include <stdio.h>
#include <filesystem>

#include "parser_helper.h"
#include "draw_helper.h"

void draw_qrcode(std::vector<DetectedBarcode>& barcodes, cv::Mat& debug_img) {
    for (const auto& barcode : barcodes) {
        std::vector<cv::Point> box;

        for (const auto& point : barcode.bounding_box) {
            box.push_back(cv::Point(point.x, point.y));
        }

        cv::polylines(debug_img, box, true, cv::Scalar(0, 0, 255), 2);
    }
}

void save_debug_img(cv::Mat debug_img, std::filesystem::path output_dir, std::filesystem::path output_img_path_fname) {
    char* output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/cal-debug-%s", output_dir.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, debug_img);
    printf("output image: %s\n", output_img_fname);
    free(output_img_fname);
}