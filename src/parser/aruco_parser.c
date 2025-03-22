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

std::optional<cv::Mat> aruco_parser(cv::Mat img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points) {
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);
    detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);
    return {};
}