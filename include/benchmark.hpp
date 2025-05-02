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
     */
    template <typename Func, typename... Args>
    static void measure(const std::string& name, Func&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();

        std::forward<Func>(func)(std::forward<Args>(args)...);

        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
        return;
    }
};

/**
 * @brief Classe RAII simplifiée pour le benchmark avec sortie CSV sans état de succès
 */
class BenchmarkGuard {
  public:
    /**
     * @brief Constructeur qui démarre le chronomètre du benchmark
     *
     * @param name Nom du benchmark à afficher dans les résultats
     * @param csv Pointeur vers un flux de fichier de sortie pour les données CSV
     */
    BenchmarkGuard(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {
    }

    float end() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        printf("%s: %.3f milliseconds\n", name_.c_str(), milliseconds);

        return milliseconds;
    }

  protected:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};
