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
#include "math_helper.h"
#include "draw_helper.h"

#include "qrcode_parser.h"

static bool compare_area(const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
    return a.first < b.first;
}

int identify_corner(std::vector<DetectedBarcode>& detected_qrcode, std::vector<cv::Point2f>& corner_points,
                    DetectedBarcode& corner_barcode) {
    corner_points.resize(4);

    int found_mask = 0x00;

    corner_points[BOTTOM_RIGHT] = center_of_box(corner_barcode.bounding_box);
    found_mask |= (1 << BOTTOM_RIGHT);

    std::pair<float, cv::Point> max_distance = { 0, cv::Point(0, 0) };

    for (const auto& qr : detected_qrcode) {
        auto center = center_of_box(qr.bounding_box);
        float x = center.x - corner_points[BOTTOM_RIGHT].x;
        float y = center.y - corner_points[BOTTOM_RIGHT].y;
        float distance = sqrt(pow(x, 2) + pow(y, 2));
        if (distance > max_distance.first) {
            max_distance = { distance, center };
        }
    }

    corner_points[TOP_LEFT] = max_distance.second;
    found_mask |= (1 << TOP_LEFT);

    std::vector<std::pair<float, cv::Point2f>> right_corner_points;

    for (const auto& qrcode : detected_qrcode) {
        auto center = center_of_box(qrcode.bounding_box);
        if (center == max_distance.second && center == cv::Point(corner_points[BOTTOM_RIGHT]))
            continue;

        float angle_corner =
            angle(corner_points[TOP_LEFT], cv::Point2f(center.x, center.y), corner_points[BOTTOM_RIGHT]);
        angle_corner = angle_corner < 0 ? angle_corner + M_PI : angle_corner;
        angle_corner = abs(angle_corner - M_PI / 2);
        right_corner_points.push_back({ angle_corner, cv::Point2f(center.x, center.y) });
    }

    std::sort(right_corner_points.begin(), right_corner_points.end(), compare_area);

    corner_points[TOP_RIGHT] = right_corner_points[0].second;
    found_mask |= (1 << TOP_RIGHT);

    corner_points[BOTTOM_LEFT] = right_corner_points[1].second;
    found_mask |= (1 << BOTTOM_LEFT);

    if (corner_points[TOP_RIGHT].x < corner_points[BOTTOM_LEFT].x) {
        std::swap(corner_points[TOP_RIGHT], corner_points[BOTTOM_LEFT]);
    }

    return found_mask;
}

std::optional<cv::Mat> main_qrcode_empty(cv::Mat img,
#ifdef DEBUG
                                         cv::Mat debug_img,
#endif
                                         Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {

    auto barcodes = identify_barcodes(img,
#ifdef ENABLE_ZBAR
                                      zbar::ZBAR_QRCODE
#else
                                      ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::MicroQRCode
#endif
    );

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
    int found_corner_mask = identify_corner(barcodes, corner_points, corner_barcode);

    if (found_corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("not all corner barcodes were found");

    meta = parse_metadata(corner_barcode.content);

    auto affine_transform = get_affine_transform(found_corner_mask, dst_corner_points, corner_points);
    return affine_transform;
}