#ifndef ARUCO_PARSER_H
#define ARUCO_PARSER_H

std::optional<cv::Mat> aruco_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);

#endif