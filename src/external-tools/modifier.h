/**
 * @file modifier.h
 * @brief Déclarations des fonctions de modification d'image (bruit, contraste, taches, transformations géométriques).
 */
 #ifndef MODIFIER_H
 #define MODIFIER_H
 
 #include "modifier_constants.h"
 #include <opencv2/opencv.hpp>
 
 /**
  * @brief Ajoute du bruit sel et poivre à une image.
  *
  * @param img        Image d'entrée à modifier (CV_8UC1 ou CV_8UC3).
  * @param rng        Générateur aléatoire OpenCV (cv::RNG).
  * @param max_pepper Pourcentage maximal de pixels assombris (poivre) à appliquer (0-100).
  * @param max_salt   Pourcentage maximal de pixels éclaircis (sel) à appliquer (0-100).
  */
 void add_salt_pepper_noise(cv::Mat& img, cv::RNG rng, float max_pepper, float max_salt);
 
 /**
  * @brief Ajoute du bruit gaussien à une image.
  *
  * @param img        Image d'entrée à modifier.
  * @param rng        Générateur aléatoire OpenCV (cv::RNG).
  * @param dispersion Écart type du bruit, exprimé en pourcentage de la plage de valeurs de l'image (0-100).
  * @param offset     Valeur moyenne du bruit, exprimée en pourcentage de la plage de valeurs de l'image (0-100).
  */
 void add_gaussian_noise(cv::Mat& img, cv::RNG rng, int dispersion, int offset);
 
 /**
  * @brief Modifie le contraste et la luminosité d'une image.
  *
  * @param img       Image d'entrée à modifier.
  * @param contrast  Coefficient de contraste :
  *                  - 100 => pas de changement
  *                  - >100 => augmentation du contraste
  *                  - <100 (>=0) => réduction du contraste
  * @param bright    Ajustement de la luminosité :
  *                  - 0 => neutre
  *                  - >0 => éclaircit
  *                  - <0 => assombrit
  */
 void contrast_brightness_modifier(cv::Mat& img, int contrast, int bright);
 
 /**
  * @brief Ajoute des taches (spots) de couleur uniforme sur une image.
  *
  * @param image        Image d'entrée à modifier.
  * @param rng          Générateur aléatoire OpenCV (cv::RNG).
  * @param nombreTaches Nombre de taches à dessiner.
  * @param rayonMin     Rayon minimal de chaque tache (en pixels).
  * @param rayonMax     Rayon maximal de chaque tache (en pixels).
  */
 void add_spot(cv::Mat& image, cv::RNG rng, int nombreTaches, int rayonMin, int rayonMax);
 
 /**
  * @brief Pivote une image autour de son centre.
  *
  * @param img    Image d'entrée à faire pivoter.
  * @param deg    Angle de rotation en degrés (positif => sens trigonométrique).
  */
 void rotate_img(cv::Mat& img, float deg);
 
 /**
  * @brief Translate une image selon un décalage horizontal et vertical.
  *
  * @param img    Image d'entrée à déplacer.
  * @param dx     Décalage horizontal en pixels (>0 => vers la droite, <0 => vers la gauche).
  * @param dy     Décalage vertical en pixels (>0 => vers le bas, <0 => vers le haut).
  */
 void translate_img(cv::Mat& img, int dx, int dy);
 
 /**
  * @brief Exécute une suite aléatoire de transformations (rotation, translation, bruit, contraste, taches).
  *
  * @param img                 Image d'entrée à modifier.
  * @param modification_matrix Matrice affine de la transformation géométrique appliquée (2x3).
  * @param seed                Graine pour le générateur aléatoire (0 => utilise l'heure actuelle).
  */
 void random_exec(cv::Mat& img, cv::Mat& modification_matrix, int seed=0);
 
 /**
  * @brief Applique une transformation combinée dépendant d'un coefficient aléatoire (distorsion géométrique et bruit).
  *
  * @param img                 Image d'entrée à modifier.
  * @param modification_matrix Matrice affine résultante de la transformation géométrique (2x3).
  * @param coef                Coefficient d’intensité des distorsions et du bruit (0.0 à 1.0).
  */
 void distorsion_coef_exec(cv::Mat& img, cv::Mat& modification_matrix, float coef);
 
 #endif // MODIFIER_H
 