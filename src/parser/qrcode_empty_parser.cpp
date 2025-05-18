#include <vector>
#include <string>

#ifdef ENABLE_ZBAR
#include <zbar.h>
#else
#include <ZXing/ReadBarcode.h>
#endif

#include <common.h>
#include "json_helper.h"
#include "string_helper.h"
#include "parser_helper.h"
#include "math_utils.h"
#include "draw_helper.h"

#include "zxing_parser.h"

std::optional<cv::Mat> qrcode_empty_parser(const cv::Mat& img,
#ifdef DEBUG
                                           cv::Mat debug_img,
#endif
                                           Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                           int flag_barcode) {

    auto barcodes = identify_barcodes(img, (ZXing::BarcodeFormat) flag_barcode);

#ifdef DEBUG
    draw_qrcode(barcodes, debug_img);
#endif

    if (barcodes.size() < 4) {
        printf("no barcode found\n");
        meta.id = 0;
        meta.page = 1;
        meta.name = "";
        return cv::Mat::eye(2, 3, CV_32F);
        // throw std::invalid_argument("no barcode found");
    }

    auto corner_barcode_opt = select_bottom_right_corner(barcodes);

    if (!corner_barcode_opt) {
        printf("no corner barcode found\n");
        return {};
    }

    auto corner_barcode = corner_barcode_opt.value();

    std::vector<cv::Point2f> corner_points;
    std::vector<cv::Point2f> barcode_pos;

    for (const auto& barcode : barcodes) {
        barcode_pos.push_back(center_of_box(barcode.bounding_box));
    }

    int found_corner_mask = found_other_point(barcode_pos, corner_points, center_of_box(corner_barcode.bounding_box));

    if (found_corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("not all corner barcodes were found");

    meta = parse_metadata(corner_barcode.content);

    auto affine_transform = get_affine_transform(found_corner_mask, dst_corner_points, corner_points);
    return affine_transform;
}