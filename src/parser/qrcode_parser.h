#ifndef QRCODE_PARSER_H
#define QRCODE_PARSER_H

std::optional<cv::Mat> main_qrcode(cv::Mat img, Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);
void draw_qrcode(cv::Mat& calibrated_img_col, const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                 const cv::Point2f& src_img_size, const cv::Point2f& dimension);
#endif // QRCODE_PARSER_H