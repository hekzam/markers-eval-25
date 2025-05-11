#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <iomanip>
#include <fstream>
#include "common.h"
#include "parser_helper.h"

/**
 * @brief Classe utilitaire pour mesurer le temps d'exécution des fonctions
 *
 * Fournit des méthodes statiques pour mesurer le temps d'exécution des fonctions
 */
class Benchmark {
  public:
    /**
     * @brief Mesure le temps d'exécution d'une fonction
     *
     * @param name Le nom de la fonction à afficher dans les résultats
     * @param func La fonction à mesurer
     * @param args Arguments à passer à la fonction
     *
     * @tparam Func Type de la fonction
     * @tparam Args Types des arguments
     * @return double Le temps d'exécution en millisecondes
     */
    template <typename Func, typename... Args>
    static double measure(const std::string& name, Func&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();

        std::forward<Func>(func)(std::forward<Args>(args)...);

        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
        return milliseconds;
    }
};
