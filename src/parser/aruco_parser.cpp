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
#include <string_helper.h>

int identify_corner_aruco(std::vector<int> markerIds, std::vector<std::vector<cv::Point2f>> markerCorners,
                          std::vector<cv::Point2f>& corner_points) {
    corner_points.resize(4);
    int found_mask = 0x00;

    for (int i = 0; i < markerIds.size(); i++) {
        int pos_found = 0;

        switch (markerIds[i]) {
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

        cv::Mat mean_mat;
        cv::reduce(markerCorners[i], mean_mat, 1, cv::REDUCE_AVG);
        corner_points[pos_found] = cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));
        int pos_found_bf = 1 << pos_found;
        // printf("found pos=%d -> bf=%d\n", pos_found, pos_found_bf);
        found_mask |= pos_found_bf;
    }

    return found_mask;
}

std::optional<cv::Mat> aruco_parser(cv::Mat img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {

    auto barcodes = identify_barcodes(img);

    if (barcodes.empty()) {
        printf("no barcode found\n");
        return {};
    }

#ifdef DEBUG
    for (const auto& barcode : barcodes) {
        std::vector<cv::Point> box;

        for (const auto& point : barcode.bounding_box) {
            box.push_back(cv::Point(point.x, point.y));
        }

        cv::polylines(debug_img, box, true, cv::Scalar(0, 0, 255), 2);
    }
#endif

    DetectedBarcode corner_barcode;

    for (const auto& barcode : barcodes) {
        if (starts_with(barcode.content, "hzbr")) {
            corner_barcode = barcode;
            break;
        }
    }
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);
    detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);

#ifdef DEBUG
    for (int i = 0; i < markerIds.size(); i++) {
        auto markerCorner = markerCorners[i];
        std::vector<cv::Point> raster_box;
        for (const auto& point : markerCorner) {
            raster_box.push_back(cv::Point(point.x, point.y));
        }
        cv::polylines(debug_img, raster_box, true, cv::Scalar(0, 255, 0), 2);
        cv::Mat mean_mat;
        cv::reduce(markerCorner, mean_mat, 1, cv::REDUCE_AVG);
        cv::putText(debug_img, std::to_string(markerIds[i]),
                    cv::Point(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1)), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(50, 50, 50), 2);
    }
#endif
    std::vector<cv::Point2f> corner_points;
    auto found_mask = identify_corner_aruco(markerIds, markerCorners, corner_points);

    if (found_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF)) {
        printf("not all corner markers found\n");
        return {};
    }

    cv::Mat mean_mat;
    cv::reduce(corner_barcode.bounding_box, mean_mat, 1, cv::REDUCE_AVG);
    corner_points[BOTTOM_RIGHT] = { cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1)) };
    found_mask |= BOTTOM_RIGHT_BF;

    meta = parse_metadata(corner_barcode.content);

    auto affine_transform = get_affine_transform(found_mask, dst_corner_points, corner_points);
    return affine_transform;
}