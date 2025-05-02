#include <common.h>
#include <random>
#include "utils/math_utils.h"

void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt) {
    int amount1 = img.rows * img.cols * max_pepper / 100; // /100 pour passer un pourcentage entier en paramètre
    int amount2 = img.rows * img.cols * max_salt / 100;
    for (int counter = 0; counter < amount1; ++counter) {
        img.at<cv::Vec3b>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = cv::Vec3b(0, 0, 0);
    }
    for (int counter = 0; counter < amount2; ++counter) {
        img.at<cv::Vec3b>(rng.uniform(0, img.rows), rng.uniform(0, img.cols)) = cv::Vec3b(255, 255, 255);
    }
}

/**
 * @brief Ajout de bruit gaussien
 *
 * @param img Image à modifier
 * @param contrast contrast = 1 : Pas de changement; contrast > 1 : Augmente le contraste; 0 < contrast < 1 : Réduit le
 * contraste
 * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
 *
 */

void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset) {
    cv::Mat noise = cv::Mat::zeros(img.size(), CV_32FC(img.channels()));
    rng.fill(noise, cv::RNG::NORMAL, percentage_to_dispersion(img.depth(), dispersion),
             percentage_to_offset(img.depth(), offset));
    cv::Mat img_float;
    img.convertTo(img_float, CV_32F);
    img_float += noise;
    img_float.convertTo(img, img.type());
}

/**
 * @brief Modification contraste et luminosité d'une image
 *
 * @param img Image à modifier
 * @param contrast contrast = 100 : Pas de changement; contrast > 100 : Augmente le contraste; 0 < contrast < 100 :
 * Réduit le contraste
 * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
 *
 */

void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright) {
    if (contrast > 0 && contrast <= 100)
        contrast += 50;
    contrast = std::max(-100, std::min(100, contrast));
    bright = std::max(-100, std::min(100, bright));

    float alpha = 1.0f + (float(contrast) / 100.0f); // [0.0, 2.0]
    float beta = float(bright) * 1.3f;               // [-130, 130]

    img.convertTo(img, -1, alpha, beta);
}

void add_ink_stain(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax) {
    for (int i = 0; i < nombreTaches; i++) {
        cv::Point centre(rng.uniform(rayonMax, image.cols - rayonMax), rng.uniform(rayonMax, image.rows - rayonMax));
        int rayon = rng.uniform(rayonMin, rayonMax);
        cv::Scalar color = cv::Scalar(1, 1, 1);
        cv::circle(image, centre, rayon, color, -1);
    }
}

void rotate_img(cv::Mat& img, float deg) {
    cv::Mat img_out = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    identity *= rotate_center(deg, img_out.cols / 2, img_out.rows / 2);
    identity = identity(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, identity, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));
}

void translate_img(cv::Mat& img, int dx, int dy) {
    cv::Mat img_out = img.clone();
    cv::Mat affine = (cv::Mat_<float>(2, 3) << 1, 0, dx, 0, 1, dy);
    cv::warpAffine(img_out, img, affine, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
}

void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed = 0) {
    cv::RNG rng;
    if (seed)
        rng = cv::RNG(seed);
    else
        rng = cv::RNG(time(0));

    // expend image
    cv::Mat img_out = img.clone();
    int percent = 30;
    cv::copyMakeBorder(img_out, img_out, percent, percent, percent, percent, cv::BORDER_CONSTANT,
                       cv::Scalar(255, 255, 255));
    // rotate_img(img, rng.uniform(-2.0, 2.0));
    // translate_img(img, rng.uniform(-5, 5), rng.uniform(-5, 5));
    modification_matrix = cv::Mat::eye(3, 3, CV_32F);
    modification_matrix *= rotate_center(rng.uniform(-2.0, 2.0), img_out.cols / 2, img_out.rows / 2);
    modification_matrix *= translate(rng.uniform(-5, 5), rng.uniform(-5, 5));
    modification_matrix = modification_matrix(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, modification_matrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));
    add_salt_pepper_noise(img, rng, rng.uniform(0.01, 0.13), rng.uniform(0.01, 0.13));
    add_gaussian_noise(img, rng, rng.uniform(1.0, 5.0), rng.uniform(1.0, 5.0));
    contrast_brightness_modifier(img, rng.uniform(-20, 20), rng.uniform(-20, 20));
    add_ink_stain(img, rng, rng.uniform(0, 8), rng.uniform(4, 8), rng.uniform(9, 35));
}