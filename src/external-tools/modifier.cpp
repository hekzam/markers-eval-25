#include <common.h>
#include <random>
#include <iostream>
#include "utils/math_utils.h"
#include "modifier_constants.h"
#include "modifier.h"


void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt) {
    int amount1 = img.rows * img.cols * max_pepper / 100;
    int amount2 = img.rows * img.cols * max_salt / 100;
    for (int counter = 0; counter < amount1; ++counter) {
        if (img.channels() == 1) {
            img.at<uchar>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = 0;
        } else if (img.channels() == 3) {
            img.at<cv::Vec3b>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = cv::Vec3b(0, 0, 0);
        }
    }
    for (int counter = 0; counter < amount2; ++counter) {
        if (img.channels() == 1) {
            img.at<uchar>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = 255;
        } else if (img.channels() == 3) {
            img.at<cv::Vec3b>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = cv::Vec3b(255, 255, 255);
        }
    }
}

/*****************************************************************************************************************************************************************/

void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset) {
    cv::Mat noise = cv::Mat::zeros(img.size(), CV_32FC(img.channels()));
    rng.fill(noise, cv::RNG::NORMAL, percentage_to_dispersion(img.depth(), dispersion),
             percentage_to_offset(img.depth(), offset));
    cv::Mat img_float;
    img.convertTo(img_float, CV_32F);
    img_float += noise;
    img_float.convertTo(img, img.type());
}

/*****************************************************************************************************************************************************************/

void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright) {
    if (contrast > 0 && contrast <= 100)
        contrast += 50;
    contrast = std::max(-100, std::min(100, contrast));
    bright = std::max(-100, std::min(100, bright));

    float alpha = 1.0f + (float(contrast) / 100.0f); 
    float beta = float(bright) * 1.3f;               

    img.convertTo(img, -1, alpha, beta);
}

/*****************************************************************************************************************************************************************/

void add_spot(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax) {
    for (int i = 0; i < nombreTaches; i++) {
        cv::Point centre(rng.uniform(rayonMax, image.cols - rayonMax), rng.uniform(rayonMax, image.rows - rayonMax));
        int rayon = rng.uniform(rayonMin, rayonMax);
        cv::Scalar color = cv::Scalar(1, 1, 1);
        cv::circle(image, centre, rayon, color, -1);
    }
}

/*****************************************************************************************************************************************************************/

void rotate_img(cv::Mat& img, float deg) {
    cv::Mat img_out = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    identity *= rotate_center(deg, img_out.cols / 2, img_out.rows / 2);
    identity = identity(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, identity, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));
}

/*****************************************************************************************************************************************************************/

void translate_img(cv::Mat& img, int dx, int dy) {
    cv::Mat img_out = img.clone();
    cv::Mat affine = (cv::Mat_<float>(2, 3) << 1, 0, dx, 0, 1, dy);
    cv::warpAffine(img_out, img, affine, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
}

void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef){
    int percent = 30;
    float neg_value=1*coef;
    if(time(0)%2==0){
        neg_value=-neg_value;
    }
    cv::RNG rng = cv::RNG(time(0));
    cv::copyMakeBorder(img, img, percent, percent, percent, percent, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    cv::Mat img_out = img.clone();
    modification_matrix = cv::Mat::eye(3, 3, CV_32F);
    modification_matrix *= rotate_center(neg_value*70, img_out.cols / 2, img_out.rows / 2);
    modification_matrix *= translate(neg_value*50, neg_value*50);
    modification_matrix = modification_matrix(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, modification_matrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    add_salt_pepper_noise(img, rng, coef*50, coef*50);
    add_gaussian_noise(img, rng, coef*50, coef*50);
    contrast_brightness_modifier(img, neg_value*100, neg_value*100);
    add_spot(img, rng, coef*MAX_NB_SPOT, rng.uniform(4, 8), rng.uniform(9, 35));
}

/*****************************************************************************************************************************************************************/

void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed) {
    cv::RNG rng;
    if (seed)
        rng = cv::RNG(seed);
    else
        rng = cv::RNG(time(0));
    // expand image
    int percent = 30;
    cv::copyMakeBorder(img, img, percent, percent, percent, percent, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    cv::Mat img_out = img.clone();
    modification_matrix = cv::Mat::eye(3, 3, CV_32F);
    modification_matrix *= rotate_center(rng.uniform(MIN_ROTATE, MAX_ROTATE), img_out.cols / 2, img_out.rows / 2);
    modification_matrix *= translate(rng.uniform(MIN_TRANS, MAX_TRANS), rng.uniform(MIN_TRANS, MAX_TRANS));
    modification_matrix = modification_matrix(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, modification_matrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,cv::Scalar(255, 255, 255));
    add_salt_pepper_noise(img, rng, rng.uniform(MIN_SALT, MAX_SALT), rng.uniform(MIN_PEPPER, MAX_PEPPER));
    add_gaussian_noise(img, rng, rng.uniform(MIN_DISP, MAX_DISP), rng.uniform(MIN_OFFSET, MAX_OFFSET));
    contrast_brightness_modifier(img, rng.uniform(MIN_CONTRAST, MAX_CONTRAST), rng.uniform(MIN_BRIGHT, MAX_BRIGHT));
    add_spot(img, rng, rng.uniform(MIN_NB_SPOT, MAX_NB_SPOT), rng.uniform(MIN_RMIN, MAX_RMIN), rng.uniform(MIN_RMAX, MAX_RMAX));
}