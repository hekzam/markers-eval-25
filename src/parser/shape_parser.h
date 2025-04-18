#ifndef SHAPE_PARSER_H
#define SHAPE_PARSER_H

std::optional<cv::Mat> shape_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);

#endif