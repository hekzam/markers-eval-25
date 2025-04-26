#ifndef PARSING_TIME_H
#define PARSING_TIME_H

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

/**
 * @file parsing_time.h
 * @brief Module pour l'évaluation des performances de la détection et du traitement des copies.
 *
 * Ce fichier contient les déclarations des fonctions utilisées pour évaluer les performances
 * de détection et de traitement des copies numérisées dans le cadre d'un benchmark.
 * Il permet de mesurer le temps d'exécution et de vérifier le bon fonctionnement des algorithmes
 * de détection des marqueurs et des zones d'intérêt sur les copies.
 */

/**
 * @brief Exécute le benchmark de performance sur un ensemble de copies.
 *
 * Cette fonction analyse les images de copies numérisées, détecte les marqueurs et les zones
 * d'intérêt, mesure le temps d'exécution et enregistre les résultats.
 *
 * @param config Un dictionnaire contenant les paramètres de configuration du benchmark.
 */
void parsing_benchmark(const std::unordered_map<std::string, Config>& config);

#endif // PARSING_TIME_H