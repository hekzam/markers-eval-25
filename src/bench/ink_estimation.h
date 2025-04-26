#ifndef INK_ESTIMATION_H
#define INK_ESTIMATION_H

/**
 * @file ink_estimation.h
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
 * @brief Configuration par défaut pour le benchmark d'estimation d'encre
 */
std::vector<std::pair<std::string, Config>> ink_estimation_config = {
    { "input-dir", { "Input directory", "The directory containing the input image", "./copies" } },
    { "marker-config",
      { "Marker configuration", "The configuration of the markers to use",
        "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)" } },
    { "encoded-marker_size", { "Encoded marker size", "The size of the encoded markers", 13 } },
    { "unencoded-marker_size", { "Unencoded marker size", "The size of the unencoded markers", 10 } },
    { "header-marker_size", { "Header marker size", "The size of the header marker", 7 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "dpi", { "DPI", "The resolution in dots per inch", 300 } },
};

/**
 * @brief Exécute le benchmark d'estimation de consommation d'encre
 *
 * @param config unordered_map contenant la configuration du benchmark
 */
void ink_estimation_benchmark(const std::unordered_map<std::string, Config>& config);

#endif // INK_ESTIMATION_H
