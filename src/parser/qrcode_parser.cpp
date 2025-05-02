#include <vector>
#include <string>

#include <common.h>
#include "json_helper.h"
#include "string_helper.h"
#include "parser_helper.h"
#include "draw_helper.h"

#include "qrcode_parser.h"

int identify_corner_barcodes(std::vector<DetectedBarcode>& barcodes, const std::string& content_hash,
                             std::vector<cv::Point2f>& corner_points, std::vector<DetectedBarcode*>& corner_barcodes) {
    corner_points.resize(4);
    corner_barcodes.resize(4);
    int found_mask = 0x00;

    for (auto& barcode : barcodes) {
        // should contain "hzXY" with XY in {tr, tr, br} or a content hash longer than 4
        if (barcode.content.size() < 4)
            continue;

        const char* s = barcode.content.c_str();
        int pos_found = 0;

        if (!starts_with(barcode.content, "hz"))
            continue;

        if (starts_with(barcode.content, "hztl")) {
            pos_found = TOP_LEFT;
        } else if (starts_with(barcode.content, "hztr")) {
            pos_found = TOP_RIGHT;
        } else if (starts_with(barcode.content, "hzbl")) {
            pos_found = BOTTOM_LEFT;
        } else if (starts_with(barcode.content, "hzbr")) {
            pos_found = BOTTOM_RIGHT;
        } else {
            continue;
        }

        cv::Mat mean_mat;
        cv::reduce(barcode.bounding_box, mean_mat, 1, cv::REDUCE_AVG);
        corner_points[pos_found] = cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));
        corner_barcodes[pos_found] = &barcode;
        int pos_found_bf = 1 << pos_found;
        // printf("found pos=%d -> bf=%d\n", pos_found, pos_found_bf);
        found_mask |= pos_found_bf;
    }

    return found_mask;
}

std::vector<DetectedBarcode> get_barcodes(const cv::Mat& img) {
    return identify_barcodes(img);
}

int nb_corner_found(int found_mask) {
    int nb_found = 0;
    for (int i = 0; i < 4; ++i) {
        if ((1 << i) & found_mask)
            nb_found++;
    }
    return nb_found;
}

std::optional<cv::Mat> qrcode_parser(const cv::Mat& img,
#ifdef DEBUG
                                     cv::Mat debug_img,
#endif
                                     Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {
    std::string expected_content_hash = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=";

    auto barcodes = identify_barcodes(img, (ZXing::BarcodeFormat) flag_barcode);
    // auto barcodes = smaller_parse(img, get_barcodes);
#ifdef DEBUG
    draw_qrcode(barcodes, debug_img);
#endif

    std::vector<cv::Point2f> corner_points;
    std::vector<DetectedBarcode*> corner_barcodes;
    int found_corner_mask = identify_corner_barcodes(barcodes, expected_content_hash, corner_points, corner_barcodes);

    if (nb_corner_found(found_corner_mask) < 3) {
        printf("need at least 3 corner barcodes\n");
        return {};
    }

    meta = parse_metadata(corner_barcodes[BOTTOM_RIGHT]->content);

    auto affine_transform = get_affine_transform(found_corner_mask, dst_corner_points, corner_points);
    return affine_transform;
}
