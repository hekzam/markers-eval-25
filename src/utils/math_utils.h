#ifndef math_utils_H
#define math_utils_H

cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size);
cv::Point2f center_of_box(std::vector<cv::Point2f> bounding_box);
float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c);
int found_other_point(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_points,
                      cv::Point2f corner_barcode);
int sum_mask(int mask);
cv::Mat translate(float x, float y);
cv::Mat rotate(float angle);
cv::Mat rotate_center(float angle, float cx, float cy);
void print_mat(cv::Mat mat);

#endif