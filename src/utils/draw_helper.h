#ifndef DRAW_HELPER_H
#define DRAW_HELPER_H

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <common.h>

void draw_qrcode(std::vector<DetectedBarcode>& barcodes, cv::Mat& debug_img);
void save_debug_img(cv::Mat debug_img, std::filesystem::path output_dir, std::filesystem::path output_img_path_fname);

#endif