#ifndef CONFIG_ANALYSIS_H
#define CONFIG_ANALYSIS_H

/**
 * @file config_analysis.h
 * @brief Benchmark pour l'estimation de la consommation d'encre.
 *
 * Ce fichier contient les fonctions pour mesurer et estimer la consommation
 * d'encre d'images en niveaux de gris en se basant sur la couverture d'encre.
 */

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <common.h>

/**
 * @brief Exécute le benchmark d'estimation de consommation d'encre
 *
 * @param config unordered_map contenant la configuration du benchmark
 */
void config_analysis_benchmark(const std::unordered_map<std::string, Config>& config);

#endif // CONFIG_ANALYSIS_H
