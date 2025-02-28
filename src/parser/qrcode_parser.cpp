#include <vector>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <ZXing/ReadBarcode.h>

#include <common.h>
#include <json_helper.h>
#include "qrcode_parser.h"
#include <string_helper.h>

struct DetectedBarcode {
    std::string content;
    std::vector<cv::Point2f> bounding_box;
};

std::vector<DetectedBarcode> identify_barcodes(cv::Mat img) {
    std::vector<DetectedBarcode> barcodes = {};

    if (img.type() != CV_8U)
        throw std::invalid_argument(
            "img has type != CV_8U while it should contain luminance information on 8-bit unsigned integers");

    if (img.cols < 2 || img.rows < 2)
        return {};

    auto iv =
        ZXing::ImageView(reinterpret_cast<const uint8_t*>(img.ptr()), img.cols, img.rows, ZXing::ImageFormat::Lum);
    auto z_barcodes = ZXing::ReadBarcodes(iv);

    for (const auto& b : z_barcodes) {
        DetectedBarcode barcode;
        barcode.content = b.text();

        std::vector<cv::Point> corners;
        for (int j = 0; j < 4; ++j) {
            const auto& p = b.position()[j];
            barcode.bounding_box.emplace_back(cv::Point2f(p.x, p.y));
        }
        barcodes.emplace_back(barcode);
    }

    return barcodes;
}

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
            if (barcode.content.find(content_hash) == std::string::npos)
                continue;
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

cv::Mat get_affine_transform(int found_corner_mask, const std::vector<cv::Point2f>& expected_corner_points,
                             const std::vector<cv::Point2f>& found_corner_points) {
    int nb_found = 0;
    std::vector<cv::Point2f> src, dst;
    src.reserve(3);
    dst.reserve(3);

    for (int corner = 0; corner < 4; ++corner) {
        if ((1 << corner) & found_corner_mask) {
            src.emplace_back(found_corner_points[corner]);
            dst.emplace_back(expected_corner_points[corner]);

            nb_found += 1;
            if (nb_found >= 3)
                break;
        }
    }

    if (nb_found != 3)
        throw std::invalid_argument("only " + std::to_string(nb_found) + " corners were found (3 or more required)");

    /*for (int i = 0; i < 3; ++i) {
    printf("src[%d]: (%f, %f)\n", i, src[i].x, src[i].y);
    printf("dst[%d]: (%f, %f)\n", i, dst[i].x, dst[i].y);
    }*/
    return cv::getAffineTransform(src, dst);
}

cv::Mat main_qrcode(cv::Mat img, Metadata& meta, std::vector<cv::Point2f> dst_corner_points) {
    std::string expected_content_hash = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=";

    auto barcodes = identify_barcodes(img);

    std::vector<cv::Point2f> corner_points;
    std::vector<DetectedBarcode*> corner_barcodes;
    int found_corner_mask = identify_corner_barcodes(barcodes, expected_content_hash, corner_points, corner_barcodes);

    meta = parse_metadata(corner_barcodes[BOTTOM_LEFT]->content);

    auto affine_transform = get_affine_transform(found_corner_mask, dst_corner_points, corner_points);
    return affine_transform;
}