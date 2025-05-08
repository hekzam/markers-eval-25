/**
 * @file default_parser.cpp
 * @brief Default parser implementation. (do nothing)
 *
 */

#include <vector>
#include <string>

#include <opencv2/aruco.hpp>

#include <common.h>
#include "json_helper.h"
#include "parser_helper.h"
#include "string_helper.h"
#include "math_utils.h"
#include "draw_helper.h"

int identify_corner_aruco(std::vector<std::pair<int, std::vector<cv::Point2f>>>& marker_aruco,
                          std::vector<cv::Point2f>& corner_points) {
    corner_points.resize(4);
    int found_mask = 0x00;

    for (int i = 0; i < marker_aruco.size(); i++) {
        int pos_found = 0;

        switch (marker_aruco[i].first) {
            case 190:
                pos_found = TOP_LEFT;
                break;
            case 997:
                pos_found = TOP_RIGHT;
                break;
            case 999:
                pos_found = BOTTOM_LEFT;
                break;
            default:
                continue;
        }

        corner_points[pos_found] = center_of_box(marker_aruco[i].second);
        int pos_found_bf = 1 << pos_found;
        found_mask |= pos_found_bf;
    }

    return found_mask;
}

std::vector<std::pair<int, std::vector<cv::Point2f>>> identify_aruco(const cv::Mat& img, const cv::Point2i& offset) {
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;

#if (CV_VERSION_MAJOR >= 4 && CV_VERSION_MINOR > 6)
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);
    detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);
#else
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    cv::aruco::detectMarkers(img, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
#endif

    std::vector<std::pair<int, std::vector<cv::Point2f>>> barcodes;
    for (int i = 0; i < markerIds.size(); i++) {
        auto markerCorner = markerCorners[i];
        for (auto& point : markerCorner) {
            point.x += offset.x;
            point.y += offset.y;
        }
        std::vector<cv::Point> raster_box;
        for (const auto& point : markerCorner) {
            raster_box.push_back(cv::Point(point.x, point.y));
        }
        cv::polylines(img, raster_box, true, cv::Scalar(0, 255, 0), 2);
        barcodes.emplace_back(markerIds[i], markerCorner);
    }

    return barcodes;
}

std::optional<cv::Mat> aruco_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {

    auto barcodes = identify_barcodes(img, (ZXing::BarcodeFormat) flag_barcode);

    if (barcodes.empty()) {
        printf("no barcode found\n");
        return {};
    }

#ifdef DEBUG
    draw_qrcode(barcodes, debug_img);
#endif

    auto corner_barcode_opt = select_bottom_right_corner(barcodes);

    if (!corner_barcode_opt) {
        printf("no corner barcode found\n");
        return {};
    }

    // auto marker_aruco = identify_aruco(img);
    auto marker_aruco = smaller_parse(img,
#ifdef DEBUG
                                      debug_img,
#endif
                                      identify_aruco);

    auto corner_barcode = corner_barcode_opt.value();

#ifdef DEBUG
    for (int i = 0; i < marker_aruco.size(); i++) {
        auto markerCorner = marker_aruco[i].second;
        std::vector<cv::Point> raster_box;
        for (const auto& point : markerCorner) {
            raster_box.push_back(cv::Point(point.x, point.y));
        }
        cv::polylines(debug_img, raster_box, true, cv::Scalar(0, 255, 0), 2);
        cv::Mat mean_mat;
        cv::reduce(markerCorner, mean_mat, 1, cv::REDUCE_AVG);
        cv::putText(debug_img, std::to_string(marker_aruco[i].first),
                    cv::Point(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1)), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(50, 50, 50), 2);
    }
#endif
    std::vector<cv::Point2f> corner_points;
    auto found_mask = identify_corner_aruco(marker_aruco, corner_points);

    corner_points[BOTTOM_RIGHT] = center_of_box(corner_barcode.bounding_box);
    found_mask |= BOTTOM_RIGHT_BF;

    meta = parse_metadata(corner_barcode.content);

    auto affine_transform = get_affine_transform(found_mask, dst_corner_points, corner_points);
    return affine_transform;
}