#include <iostream>
#include <common.h>
#include "utils/math_utils.h"


void add_salt_pepper_noise(cv::Mat &srcArr, float max_pepper, float max_salt)
{   
    cv::RNG rng; 
    int amount1=srcArr.rows*srcArr.cols*max_pepper;
    int amount2=srcArr.rows*srcArr.cols*max_salt;
    for(int counter=0; counter<amount1; ++counter)
    {
        srcArr.at<cv::Vec3b>(rng.uniform(0,srcArr.rows), rng.uniform(0, srcArr.cols)) = cv::Vec3b(0, 0, 0);
    }
    for (int counter=0; counter<amount2; ++counter){
        srcArr.at<cv::Vec3b>(rng.uniform(0,srcArr.rows), rng.uniform(0,srcArr.cols)) = cv::Vec3b(255, 255, 255);
    }
}


void add_gaussian_noise(cv::Mat &srcArr, double mean, double sigma)       
{
    cv::Mat NoiseArr = srcArr.clone();
    cv::RNG rng;
    rng.fill(NoiseArr, cv::RNG::NORMAL, mean,sigma);  
    add(srcArr, NoiseArr, srcArr);   
}

void brightness_modifier(c::)       //TODO



int main(int argc, char const* argv[]) 
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <image_max_pepperth>" << std::endl;
        return 1;
    }
    std::string image_max_pepperth = argv[1];
    cv::Mat img = cv::imread(image_max_pepperth);
    cv::Mat calibrated_img = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    identity *= rotate_center(5, img.cols / 2, img.rows / 2);
    //print_mat(identity);
    identity *= translate(3, 0);
    //print_mat(identity);
    identity = identity(cv::Rect(0, 0, 3, 2));
    //print_mat(identity);
    cv::Mat noisy_img = img.clone();
    add_gaussian_noise(noisy_img, 50, 50);  // Ajoute le bruit
    cv::warpAffine(noisy_img, calibrated_img, identity, noisy_img.size(), cv::INTER_LINEAR);
    cv::imwrite("calibrated_img.png",calibrated_img);
    return 0;
}