#ifndef CUSTOM_PARSER_H
#define CUSTOM_PARSER_H

std::optional<cv::Mat> custom_marker_parser(cv::Mat img, Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);

#endif