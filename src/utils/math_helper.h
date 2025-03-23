#ifndef MATH_HELPER_H
#define MATH_HELPER_H

cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size);
cv::Point2f center_of_box(std::vector<cv::Point2f> bounding_box);
float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c);
int found_other_point(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_points,
                      DetectedBarcode& corner_barcode);

#endif