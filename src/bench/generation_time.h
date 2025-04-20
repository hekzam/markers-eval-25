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
 * @brief Exécute le benchmark de performance sur la génération de copies.
 *
 * Cette fonction génère un ensemble de copies numérisées avec différentes configurations,
 * mesure le temps d'exécution et enregistre les résultats.
 *
 * @param config Un dictionnaire contenant les paramètres de configuration du benchmark.
 */
void generation_benchmark(std::map<std::string, Config> config);

#endif // TIME_GENERATION_H
