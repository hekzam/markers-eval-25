#ifndef MODIFIER_H
#define MODIFIER_H

#include "modifier_constants.h"
#include <opencv2/opencv.hpp>

#define MARGIN_COPY_MODIFIED 100

void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt);
void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset);
void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright);
void add_ink_stain(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax);
void rotate_img(cv::Mat& img, float deg);
void translate_img(cv::Mat& img, int dx, int dy);
void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef);

/**
 * @brief Exécute une série de transformations aléatoires sur une image
 * @param img Image à modifier
 * @param modification_matrix Matrice de modification retournée par référence
 * @param seed Seed pour initialiser le générateur aléatoire (si 0, utilise le timestamp actuel)
 */
void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed = 0);

#endif