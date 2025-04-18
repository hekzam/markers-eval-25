#ifndef CUSTOM_PARSER_H
#define CUSTOM_PARSER_H

/// @brief test of custom marker parser but not finish because way to hard to detect specific shape of marker
/// @param img
/// @param meta
/// @param dst_corner_points
/// @return
std::optional<cv::Mat> custom_marker_parser(const cv::Mat& img, Metadata& meta,
                                            std::vector<cv::Point2f>& dst_corner_points);

#endif