#include <common.h>
#include <random>
#include <iostream>
#include "utils/math_utils.h"
#include "modifier_constants.h"
#include "modifier.h"

#include "modifier.h"

void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt) {
    int amount1 = img.rows * img.cols * max_pepper / 100; // /100 pour passer un pourcentage entier en paramètre
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

void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef) {
    int percent = 30;
    float neg_value = 1 * coef;
    if (time(0) % 2 == 0) {
        neg_value = -neg_value;
    }
    cv::RNG rng = cv::RNG(time(0));
    cv::copyMakeBorder(img, img, percent, percent, percent, percent, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    cv::Mat img_out = img.clone();
    modification_matrix = cv::Mat::eye(3, 3, CV_32F);
    modification_matrix *= rotate_center(neg_value * 70, img_out.cols / 2, img_out.rows / 2);
    modification_matrix *= translate(neg_value * 50, neg_value * 50);
    modification_matrix = modification_matrix(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, modification_matrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));
    add_salt_pepper_noise(img, rng, coef * 50, coef * 50);
    add_gaussian_noise(img, rng, coef * 50, coef * 50);
    contrast_brightness_modifier(img, neg_value * 100, neg_value * 100);
    add_ink_stain(img, rng, coef * MAX_NB_SPOT, rng.uniform(4, 8), rng.uniform(9, 35));
}
/**
 * @brief Applique une compression JPEG à une image avec un niveau de qualité spécifié
 *
 * @param img Image à compresser
 * @param quality Qualité de la compression (0-100, où 0 est la plus mauvaise qualité et 100 la meilleure)
 */
void apply_jpeg_compression(cv::Mat& img, int quality) {
    std::vector<uchar> buffer;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);

    // Encoder l'image en JPEG dans un buffer mémoire
    cv::imencode(".jpg", img, buffer, compression_params);

    // Décoder le buffer JPEG en cv::Mat
    img = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
}

/**
 * @brief Simule des effets d'impression sur une image
 *
 * @param img Image à modifier
 * @param rng Générateur de nombres aléatoires
 * @param intensity Intensité des effets (entre 0 et 100)
 */
void simulate_printer_effects(cv::Mat& img, cv::RNG& rng, float intensity) {
    intensity = std::max(0.0f, std::min(100.0f, intensity));
    float intensity_factor = intensity / 100.0f;

    // Convertir l'image en niveaux de gris temporairement si elle est en couleur
    cv::Mat gray_img;
    bool isColor = img.channels() == 3;
    if (isColor) {
        cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
    } else {
        gray_img = img.clone();
    }

    // Appliquer systématiquement l'effet de tramage/dithering
    {
        // Effet de tramage/dithering (pattern artificiels)
        cv::Mat pattern(8, 8, CV_8UC1);
        for (int y = 0; y < pattern.rows; y++) {
            for (int x = 0; x < pattern.cols; x++) {
                // Créer un motif de tramage Floyd-Steinberg simplifié
                pattern.at<uchar>(y, x) = ((x ^ y) & 1) ? 10 : -10;
            }
        }

        // Appliquer le motif à l'image avec l'intensité donnée
        for (int y = 0; y < img.rows; y++) {
            for (int x = 0; x < img.cols; x++) {
                int pattern_val = pattern.at<uchar>(y % pattern.rows, x % pattern.cols);
                int adjustment = pattern_val * intensity_factor *
                                 0.5f; // Réduire légèrement l'intensité pour le dithering systématique

                if (isColor) {
                    cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
                    pixel[0] = cv::saturate_cast<uchar>(pixel[0] + adjustment);
                    pixel[1] = cv::saturate_cast<uchar>(pixel[1] + adjustment);
                    pixel[2] = cv::saturate_cast<uchar>(pixel[2] + adjustment);
                    img.at<cv::Vec3b>(y, x) = pixel;
                } else {
                    uchar pixel = img.at<uchar>(y, x);
                    img.at<uchar>(y, x) = cv::saturate_cast<uchar>(pixel + adjustment);
                }
            }
        }
    }

    // En plus du tramage, appliquer un effet aléatoire supplémentaire
    int effect_type = rng.uniform(1, 3); // Maintenant seulement 3 types d'effets différents (sans le dithering)

    switch (effect_type) {
        // case 0: {
        //     // Effet 1: Stries horizontales (similaires aux imprimantes jet d'encre)
        //     int num_stripes = rng.uniform(3, 15);
        //     int stripe_height = img.rows / num_stripes;

        //     for (int i = 0; i < num_stripes; i++) {
        //         int y_start = i * stripe_height;
        //         int y_end = std::min(y_start + stripe_height, img.rows);

        //         // Intensité variable par bande
        //         float stripe_factor = 1.0f - rng.uniform(0.0f, 0.3f * intensity_factor);

        //         for (int y = y_start; y < y_end; y++) {
        //             for (int x = 0; x < img.cols; x++) {
        //                 if (isColor) {
        //                     cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
        //                     pixel[0] = cv::saturate_cast<uchar>(pixel[0] * stripe_factor);
        //                     pixel[1] = cv::saturate_cast<uchar>(pixel[1] * stripe_factor);
        //                     pixel[2] = cv::saturate_cast<uchar>(pixel[2] * stripe_factor);
        //                     img.at<cv::Vec3b>(y, x) = pixel;
        //                 } else {
        //                     uchar pixel = img.at<uchar>(y, x);
        //                     img.at<uchar>(y, x) = cv::saturate_cast<uchar>(pixel * stripe_factor);
        //                 }
        //             }
        //         }
        //     }
        //     break;
        // }
        case 1: {
            // Effet 2: Variations de densité d'encre (impression non uniforme)
            // cv::Mat noise(img.size(), CV_32FC(img.channels()));
            // cv::randn(noise, 1.0, 0.2 * intensity_factor);

            // cv::Mat img_float;
            // img.convertTo(img_float, CV_32F);

            // Multiplier l'image par le bruit pour créer des variations de densité
            // cv::multiply(img_float, noise, img_float);
            // img_float.convertTo(img, img.type());
            break;
        }
        case 2: {
            // Effet 3: Lignes manquantes (simule un défaut d'impression)
            int num_lines = rng.uniform(1, 5 + int(intensity_factor * 10));

            for (int i = 0; i < num_lines; i++) {
                int y = rng.uniform(0, img.rows);
                int length = rng.uniform(img.cols / 10, img.cols / 3);
                int start_x = rng.uniform(0, img.cols - length);

                cv::line(img, cv::Point(start_x, y), cv::Point(start_x + length, y), cv::Scalar(255, 255, 255),
                         1 + rng.uniform(0, 2));
            }
            break;
        }
    }
}

void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed) {
    cv::RNG rng;
    if (seed)
        rng = cv::RNG(seed);
    else {
        seed = static_cast<int>(time(0));
        rng = cv::RNG(seed);
    }

    // expend image
    int pixel_offset = MARGIN_COPY_MODIFIED;
    cv::copyMakeBorder(img, img, pixel_offset, pixel_offset, pixel_offset, pixel_offset, cv::BORDER_CONSTANT,
                       cv::Scalar(255, 255, 255));
    cv::Mat img_out = img.clone();

    // Déterminer si l'image doit être complètement retournée (rotation à 180 degrés)
    bool flip_image = rng.uniform(0, 10) < 2; // ~20% de chance de retourner l'image

    // Calculer l'angle de rotation
    float rotation_angle;
    if (flip_image) {
        // Rotation à 180 degrés avec une légère variation de ±3 degrés
        rotation_angle = 180.0f + rng.uniform(MIN_ROTATE, MAX_ROTATE);
    } else {
        // Légère rotation comme avant
        rotation_angle = rng.uniform(MIN_ROTATE, MAX_ROTATE);
    }

    // Appliquer la rotation
    modification_matrix = cv::Mat::eye(3, 3, CV_32F);
    modification_matrix *= rotate_center(rotation_angle, img_out.cols / 2, img_out.rows / 2);
    modification_matrix *= translate(rng.uniform(MIN_TRANS, MAX_TRANS), rng.uniform(MIN_TRANS, MAX_TRANS));
    modification_matrix = modification_matrix(cv::Rect(0, 0, 3, 2));
    cv::warpAffine(img_out, img, modification_matrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(255, 255, 255));
    add_salt_pepper_noise(img, rng, rng.uniform(MIN_SALT, MAX_SALT), rng.uniform(MIN_PEPPER, MAX_PEPPER));
    add_gaussian_noise(img, rng, rng.uniform(MIN_DISP, MAX_DISP), rng.uniform(MIN_OFFSET, MAX_OFFSET));
    contrast_brightness_modifier(img, rng.uniform(MIN_CONTRAST, MAX_CONTRAST), rng.uniform(MIN_BRIGHT, MAX_BRIGHT));
    add_ink_stain(img, rng, rng.uniform(MIN_NB_SPOT, MAX_NB_SPOT), rng.uniform(MIN_RMIN, MAX_RMIN),
                  rng.uniform(MIN_RMAX, MAX_RMAX));

    // Simuler des effets d'impression avec une intensité aléatoire
    simulate_printer_effects(img, rng, rng.uniform(60, 70));

    // Ajouter une compression JPEG
    apply_jpeg_compression(img, rng.uniform(45, 55));
}