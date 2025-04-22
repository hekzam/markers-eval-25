#ifndef INK_ESTIMATION_H
#define INK_ESTIMATION_H

/**
 * @file ink_estimation.h
 * @brief Benchmark pour l'estimation de la consommation d'encre.
 *
 * Ce fichier contient les fonctions pour mesurer et estimer la consommation
 * d'encre d'images en niveaux de gris en se basant sur la couverture d'encre.
 */

#include <map>
#include <string>
#include <vector>
#include <utility>

/**
 * @brief Vérifie les contraintes pour le benchmark d'estimation d'encre
 * 
 * @param config Map contenant la configuration
 * @return true si les contraintes sont satisfaites
 * @return false sinon
 */
bool ink_estimation_constraint(const std::map<std::string, Config>& config);

/**
 * @brief Calcule la consommation d'encre estimée pour une image en niveaux de gris
 * 
 * @param image Image en niveaux de gris à analyser
 * @param dpi Résolution en DPI
 * @param calibration_factor Facteur de calibration (ml/cm² à 100% couverture)
 * @return double Volume d'encre estimé en ml
 */
double estimate_ink_consumption(const cv::Mat& image, int dpi, double calibration_factor);

/**
 * @brief Exécute le benchmark d'estimation de consommation d'encre
 * 
 * @param config Map contenant la configuration du benchmark
 */
void ink_estimation_benchmark(const std::map<std::string, Config>& config);

#endif // INK_ESTIMATION_H
