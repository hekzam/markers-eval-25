#ifndef TIME_COPY_H
#define TIME_COPY_H

/**
 * @file time_copy.h
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
void run_benchmark(std::unordered_map<std::string, Config> config);

/**
 * @brief Vérifie que les paramètres de configuration respectent les contraintes nécessaires.
 *
 * Cette fonction s'assure que les chemins de fichiers et répertoires existent et que les paramètres
 * numériques sont dans les plages acceptables.
 *
 * @param config Un dictionnaire contenant les paramètres de configuration à vérifier.
 * @return true si la configuration est valide, false sinon.
 */
bool constraint(std::unordered_map<std::string, Config> config);

#endif // TIME_COPY_H