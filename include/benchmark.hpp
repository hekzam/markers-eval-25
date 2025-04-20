#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <iomanip> // Pour std::fixed et std::setprecision
#include <fstream>

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
        // Enregistre l'heure de début
        auto start = std::chrono::high_resolution_clock::now();

        // Exécute la fonction avec les arguments fournis
        std::forward<Func>(func)(std::forward<Args>(args)...);

        // Enregistre l'heure de fin et calcule la durée
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double milliseconds = microseconds / 1000.0;

        // Affiche le temps d'exécution
        std::cout << name << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
        return;
    }
};

/**
 * @brief Macro pour faciliter le benchmark de fonctions nommées
 *
 * Utilise automatiquement le nom de la fonction comme nom du benchmark
 */
#define BENCHMARK_FUNCTION(func, ...) Benchmark::measure(#func, func, __VA_ARGS__)

/**
 * @brief Classe RAII pour le benchmark de blocs de code
 *
 * Mesure automatiquement le temps d'exécution depuis la construction jusqu'à la destruction
 */
class BenchmarkGuard {
  public:
    /**
     * @brief Constructeur qui démarre le chronomètre du benchmark
     *
     * @param name Nom du benchmark à afficher dans les résultats
     */
    BenchmarkGuard(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {
    }

    /**
     * @brief Destructeur qui calcule et affiche le temps d'exécution
     */
    ~BenchmarkGuard() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds"
                  << std::endl;
    }

  private:
    std::string name_;                                                  // Nom du benchmark
    std::chrono::time_point<std::chrono::high_resolution_clock> start_; // Temps de démarrage
};

/**
 * @brief Macro pour faciliter le benchmark de blocs de code
 *
 * Crée un objet BenchmarkGuard avec un nom unique basé sur le numéro de ligne
 */
#define BENCHMARK_BLOCK(name) BenchmarkGuard benchmark_guard##__LINE__(name)

/**
 * @brief Classe RAII étendue pour le benchmark avec sortie CSV
 *
 * Semblable à BenchmarkGuard mais écrit également les résultats dans un fichier CSV
 */
class BenchmarkGuardCSV {
  public:
    /**
     * @brief Constructeur qui démarre le chronomètre du benchmark
     *
     * @param name Nom du benchmark à afficher dans les résultats
     * @param csv Pointeur vers un flux de fichier de sortie pour les données CSV
     */
    BenchmarkGuardCSV(const std::string& name, std::ofstream* csv)
        : name_(name), csv_(csv), success_(false), start_(std::chrono::high_resolution_clock::now()) {
    }

    /**
     * @brief Définit si l'opération mesurée a réussi
     *
     * @param success True si l'opération a réussi, false sinon
     */
    void setSuccess(bool success) {
        success_ = success;
    }

    /// TODO: Utiliser un ptr

    /**
     * @brief Destructeur qui calcule et affiche le temps d'exécution
     *
     * Écrit les résultats à la fois sur la console et dans le fichier CSV si fourni
     */
    ~BenchmarkGuardCSV() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds"
                  << std::endl;
        if (csv_) {
            *csv_ << name_ << "," << std::fixed << std::setprecision(3) << milliseconds << "," << (success_ ? "1" : "0")
                  << std::endl;
        }
    }

  private:
    std::string name_;                                                  // Nom du benchmark
    std::chrono::time_point<std::chrono::high_resolution_clock> start_; // Temps de démarrage
    std::ofstream* csv_; // Flux de fichier de sortie pour les données CSV
    bool success_;       // Si l'opération mesurée a réussi
};

/**
 * @brief Macro pour faciliter le benchmark de blocs de code avec sortie CSV
 *
 * Crée un objet BenchmarkGuardCSV avec un nom unique basé sur le numéro de ligne
 */
#define BENCHMARK_BLOCK_CSV(name, csv_ptr) BenchmarkGuardCSV benchmark_guard_csv##__LINE__(name, csv_ptr)
