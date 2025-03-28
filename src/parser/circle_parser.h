#ifndef CIRCLE_PARSER_H
#define CIRCLE_PARSER_H

std::optional<cv::Mat> main_circle(cv::Mat img,
#ifdef DEBUG
                                   cv::Mat debug_img,
#endif
                                   Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);

#endif