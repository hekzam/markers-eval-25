#ifndef QRCODE_EMPTY_PARSER_H
#define QRCODE_EMPTY_PARSER_H

std::optional<cv::Mat> qrcode_empty_parser(cv::Mat img,
#ifdef DEBUG
                                           cv::Mat debug_img,
#endif
                                           Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);
#endif // QRCODE_PARSER_H