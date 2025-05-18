#ifndef MODIFIER_H
#define MODIFIER_H

/**
 * @file modifier.h
 * @brief Module de modification d'images pour simuler des conditions réelles de numérisation.
 *
 * Ce module fournit des fonctions pour appliquer différentes transformations et 
 * dégradations à une image, afin de tester la robustesse des algorithmes de reconnaissance
 * face à des conditions variées (bruit, rotation, contraste, etc.).
 */

#include "modifier_constants.h"
#include <opencv2/opencv.hpp>

#define MARGIN_COPY_MODIFIED 100

/**
 * @brief Ajoute du bruit de type "sel et poivre" à une image
 * 
 * @param img Image à modifier
 * @param rng Générateur de nombres aléatoires
 * @param max_pepper Pourcentage maximal de pixels noirs à ajouter
 * @param max_salt Pourcentage maximal de pixels blancs à ajouter
 */
void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt);

/**
 * @brief Ajout de bruit gaussien à une image
 *
 * @param img Image à modifier
 * @param rng Générateur de nombres aléatoires
 * @param dispersion Niveau de dispersion du bruit
 * @param offset Décalage pour le bruit
 */
void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset);

/**
 * @brief Modification du contraste et de la luminosité d'une image
 *
 * @param img Image à modifier
 * @param contrast Niveau de contraste (100: pas de changement, >100: augmente, <100: réduit)
 * @param bright Luminosité (-100 à 100, 0: neutre, >0: plus clair, <0: plus sombre)
 */
void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright);

/**
 * @brief Ajoute des taches d'encre aléatoires sur l'image
 *
 * @param image Image à modifier
 * @param rng Générateur de nombres aléatoires
 * @param nombreTaches Nombre de taches à ajouter
 * @param rayonMin Rayon minimal des taches
 * @param rayonMax Rayon maximal des taches
 */
void add_ink_stain(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax);

/**
 * @brief Applique une rotation à l'image autour de son centre
 *
 * @param img Image à modifier
 * @param deg Angle de rotation en degrés
 */
void rotate_img(cv::Mat& img, float deg);

/**
 * @brief Applique une translation à l'image
 *
 * @param img Image à modifier
 * @param dx Translation horizontale en pixels
 * @param dy Translation verticale en pixels
 */
void translate_img(cv::Mat& img, int dx, int dy);

/**
 * @brief Applique une distorsion à l'image avec un coefficient d'intensité
 *
 * Cette fonction combine rotation, translation, bruit et autres effets
 * avec une intensité proportionnelle au coefficient fourni.
 *
 * @param img Image à modifier
 * @param modification_matrix Matrice de modification retournée par référence
 * @param coef Coefficient d'intensité des modifications
 */
void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef);
void apply_jpeg_compression(cv::Mat& img, int quality);

void simulate_printer_effects(cv::Mat& img, cv::RNG& rng, float intensity);

/**
 * @brief Exécute une série de transformations aléatoires sur une image
 *
 * Cette fonction applique plusieurs effets aléatoires pour simuler
 * des conditions réelles de numérisation: bruit, rotation, taches,
 * compression, effets d'impression, etc.
 *
 * @param img Image à modifier
 * @param modification_matrix Matrice de modification retournée par référence
 * @param seed Seed pour initialiser le générateur aléatoire (si 0, utilise le timestamp actuel)
 */
void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed = 0);

#endif