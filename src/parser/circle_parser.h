#ifndef CIRCLE_PARSER_H
#define CIRCLE_PARSER_H

cv::Mat main_circle(cv::Mat img, Metadata& meta, std::vector<cv::Point2f> dst_corner_points);
void draw_circle(cv::Mat& calibrated_img_col, const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                 const cv::Point2f& src_img_size, const cv::Point2f& dimension);

#endif