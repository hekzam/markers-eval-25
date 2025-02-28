#ifndef QRCODE_PARSER_H
#define QRCODE_PARSER_H

cv::Mat main_qrcode(cv::Mat img, Metadata& meta, std::vector<cv::Point2f> dst_corner_points);

#endif // QRCODE_PARSER_H