#ifndef MODIFIER_H
#define MODIFIER_H

#include "modifier_constants.h"
#include <opencv2/opencv.hpp>    


void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt);
void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset);
void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright);
void add_ink_stain(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax);
void rotate_img(cv::Mat& img, float deg);
void translate_img(cv::Mat& img, int dx, int dy);
void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed=0);
void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef);

#endif