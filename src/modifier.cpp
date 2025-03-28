#include <iostream>

#include <common.h>

cv::Mat translate(float x, float y, cv::Mat mat) {
    cv::Mat out = mat.clone();
    out.at<float>(0, 2) += x;
    out.at<float>(1, 2) += y;
    return out;
}

void print_mat(cv::Mat mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            std::cout << mat.at<float>(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char const* argv[]) {
    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
    //     return 1;
    // }
    // std::string image_path = argv[1];
    std::string image_path = "../copies/copy.png";
    cv::Mat img = cv::imread(image_path);
    cv::Mat calibrated_img = img.clone();
    cv::Mat identity = cv::Mat::eye(2, 3, CV_32F);
    print_mat(identity);
    identity = cv::getRotationMatrix2D(cv::Point2f(img.cols / 2, img.rows / 2), 0.0, 1.0);
    print_mat(identity);
    identity = translate(100, 100, identity);
    print_mat(identity);
    cv::warpAffine(img, calibrated_img, identity, calibrated_img.size(), cv::INTER_LINEAR);
    cv::imwrite("calibrated_img.png", calibrated_img);
    return 0;
}
