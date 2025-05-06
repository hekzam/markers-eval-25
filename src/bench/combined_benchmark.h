#ifndef COMBINED_BENCHMARK_H
#define COMBINED_BENCHMARK_H

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

/**
 * @file combined_benchmark.h
 * @brief Module pour l'évaluation combinée des performances de génération et de parsing des copies.
 *
 * Ce fichier contient les déclarations des fonctions utilisées pour évaluer les performances
 * de génération et de détection des copies numérisées dans le cadre d'un benchmark intégré.
 * Il permet de mesurer le temps d'exécution de génération, le taux de succès du parsing,
 * et d'enregistrer ces résultats dans un même rapport CSV.
 */

/**
 * @brief Exécute le benchmark combiné de génération et parsing sur un ensemble de copies.
 *
 * Cette fonction génère des copies numérisées avec différentes configurations, 
 * puis les analyse en mesurant le temps d'exécution pour chaque étape et le taux de succès du parsing.
 * Les résultats sont enregistrés dans un fichier CSV unique contenant toutes les métriques.
 *
 * @param config Un dictionnaire contenant les paramètres de configuration du benchmark.
 */
void combined_benchmark(const std::unordered_map<std::string, Config>& config);

#endif // COMBINED_BENCHMARK_H