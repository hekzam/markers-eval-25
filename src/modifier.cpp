#include <iostream>

#include <common.h>

#include "utils/math_utils.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }
    std::string image_path = argv[1];
    cv::Mat img = cv::imread(image_path);
    cv::Mat calibrated_img = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    identity *= rotate_center(45, img.cols / 2, img.rows / 2);
    print_mat(identity);
    identity *= translate(0, 0);
    print_mat(identity);
    identity = identity(cv::Rect(0, 0, 3, 2));
    print_mat(identity);
    cv::warpAffine(img, calibrated_img, identity, calibrated_img.size(), cv::INTER_LINEAR);
    cv::imwrite("calibrated_img.png", calibrated_img);
    return 0;
}
