#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#ifdef ENABLE_ZBAR
#include <zbar.h>
#else
#include <ZXing/ReadBarcode.h>
#endif

struct DetectedBarcode {
    std::string content;
    std::vector<cv::Point2f> bounding_box;
};

std::vector<DetectedBarcode> identify_barcodes(cv::Mat img,
#ifdef ENABLE_ZBAR
                                               zbar_symbol_type_t flags = zbar::ZBAR_QRCODE
#else
                                               ZXing::BarcodeFormats flags = ZXing::BarcodeFormat::QRCode
#endif
);
cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size);
cv::Mat get_affine_transform(int found_corner_mask, const std::vector<cv::Point2f>& expected_corner_points,
                             const std::vector<cv::Point2f>& found_corner_points);
cv::Point center_of_box(std::vector<cv::Point2f> bounding_box);
void differentiate_atomic_boxes(std::vector<std::shared_ptr<AtomicBox>>& boxes,
                                std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page);
std::vector<cv::Point2f> calculate_center_of_marker(const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                                    const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);
cv::Mat redress_image(cv::Mat img, cv::Mat affine_transform);
float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c);
std::optional<cv::Mat> run_parser(const std::string& parser_name, cv::Mat img,
#ifdef DEBUG
                                  cv::Mat debug_img,
#endif
                                  Metadata& meta, std::vector<cv::Point2f>& dst_corner_points);

#endif