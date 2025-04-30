#ifndef DEFAULT_PARSER_H
#define DEFAULT_PARSER_H

std::optional<cv::Mat> default_parser(const cv::Mat& img,
#ifdef DEBUG
                                      cv::Mat debug_img,
#endif
                                      Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

#endif