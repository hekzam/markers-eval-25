#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

struct DetectedBarcode {
    std::string content;
    std::vector<cv::Point2f> bounding_box;
};

std::vector<DetectedBarcode> identify_barcodes(cv::Mat img);
cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size);
cv::Mat get_affine_transform(int found_corner_mask, const std::vector<cv::Point2f>& expected_corner_points,
                             const std::vector<cv::Point2f>& found_corner_points);

cv::Point center_of_box(std::vector<cv::Point2f> bounding_box);

#endif