/**
 * @file modifier_constants.h
 * @brief Constantes utilisées pour la modification d'images.
 *
 * Ce fichier définit les valeurs limites (min/max) pour les différents
 * types de modifications qui peuvent être appliquées aux images lors
 * des tests de robustesse des algorithmes de détection de marqueurs.
 */
#ifndef MODIFIER_CONSTANTS_H
#define MODIFIER_CONSTANTS_H

/**
 * Paramètres de rotation
 * Définissent les angles minimaux et maximaux (en degrés)
 * pour la rotation aléatoire des images.
 */
constexpr float MIN_ROTATE = -5.0f;
constexpr float MAX_ROTATE = 5.0f;

/**
 * Paramètres de translation
 * Définissent les déplacements minimaux et maximaux (en pixels)
 * pour la translation aléatoire des images.
 */
constexpr int MIN_TRANS = -5;
constexpr int MAX_TRANS = 5;

/**
 * Paramètres de bruit "poivre et sel"
 * Définissent les pourcentages minimaux et maximaux de pixels
 * blancs (sel) et noirs (poivre) à ajouter à l'image.
 */
constexpr float MIN_SALT = 0.00f;
constexpr float MAX_SALT = 0.10f;
constexpr float MIN_PEPPER = 0.00f;
constexpr float MAX_PEPPER = 0.10f;

/**
 * Paramètres de bruit gaussien
 * Définissent les valeurs minimales et maximales pour
 * l'offset (décalage) et la dispersion (intensité) du bruit gaussien.
 */
constexpr float MIN_OFFSET = 1.0f;
constexpr float MAX_OFFSET = 5.0f;
constexpr float MIN_DISP = 1.0f;
constexpr float MAX_DISP = 5.0f;

/**
 * Paramètres de contraste et luminosité
 * Définissent les variations minimales et maximales à appliquer
 * au contraste et à la luminosité des images.
 */
constexpr int MIN_CONTRAST = -10;
constexpr int MAX_CONTRAST = 10;
constexpr int MIN_BRIGHT = -5;
constexpr int MAX_BRIGHT = 5;

/**
 * Paramètres pour les taches d'encre
 * Définissent les nombre et tailles des taches d'encre
 * à ajouter sur l'image pour simuler des défauts d'impression.
 * NB_SPOT: nombre de taches
 * RMIN: rayon minimal des taches
 * RMAX: rayon maximal des taches
 */
constexpr int MIN_NB_SPOT = 0;
constexpr int MAX_NB_SPOT = 8;
constexpr int MIN_RMIN = 4;
constexpr int MAX_RMIN = 8;
constexpr int MIN_RMAX = 9;
constexpr int MAX_RMAX = 35;

#endif // MODIFIER_CONSTANTS_H
