#ifndef TIME_GENERATION_H
#define TIME_GENERATION_H

#include <map>
#include <string>

/**
 * @file generation_time.h
 * @brief Module pour l'évaluation des performances de la génération de copies.
 *
 * Ce fichier contient les déclarations des fonctions utilisées pour évaluer les performances
 * de génération de copies numérisées dans le cadre d'un benchmark.
 * Il permet de mesurer le temps d'exécution et de vérifier le bon fonctionnement des algorithmes
 * de génération de copies avec différentes configurations de marqueurs.
 */

/**
 * @brief Vérifie si la configuration répond aux contraintes pour la génération de copies.
 *
 * Cette fonction valide que la configuration fournie contient des paramètres valides
 * pour la génération des copies.
 *
 * @param config La configuration à vérifier.
 * @return true si la configuration est valide, false sinon.
 */
bool generation_constraint(const std::map<std::string, Config>& config);

/**
 * @brief Exécute le benchmark de performance sur la génération de copies.
 *
 * Cette fonction génère un ensemble de copies numérisées avec différentes configurations,
 * mesure le temps d'exécution et enregistre les résultats.
 *
 * @param config La configuration du benchmark, incluant les paramètres de génération de copies.
 */
void generation_benchmark(const std::map<std::string, Config>& config);

#endif // TIME_GENERATION_H
