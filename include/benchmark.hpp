#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <iomanip>
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
 * @brief Macro pour faciliter le benchmark de fonctions nommées
 */
#define BENCHMARK_FUNCTION(func, ...) Benchmark::measure(#func, func, __VA_ARGS__)

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
    BenchmarkGuard(const std::string& name, std::ofstream* csv)
        : name_(name), csv_(csv), start_(std::chrono::high_resolution_clock::now()) {
    }

    /**
     * @brief Destructeur qui calcule et affiche le temps d'exécution
     *
     * Écrit les résultats à la fois sur la console et dans le fichier CSV si fourni
     */
    virtual ~BenchmarkGuard() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        if (!finished_) {
            std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds"
                      << std::endl;
            if (csv_) {
                *csv_ << name_ << "," << std::fixed << std::setprecision(3) << milliseconds << std::endl;
            }
        }
    }

  protected:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    std::ofstream* csv_;
    bool finished_ = false;
};

/**
 * @brief Crée un objet BenchmarkGuard avec un nom unique basé sur le numéro de ligne
 */
#define BENCHMARK_BLOCK_SIMPLE_CSV(name, csv_ptr) BenchmarkGuard benchmark_guard##__LINE__(name, csv_ptr)

/**
 * @brief Classe RAII étendue pour le benchmark avec sortie CSV et état de succès
 */
class BenchmarkGuardSuccess : public BenchmarkGuard {
  public:
    /**
     * @brief Constructeur qui démarre le chronomètre du benchmark
     *
     * @param name Nom du benchmark à afficher dans les résultats
     * @param csv Pointeur vers un flux de fichier de sortie pour les données CSV
     */
    BenchmarkGuardSuccess(const std::string& name, std::ofstream* csv) : BenchmarkGuard(name, csv), success_(false) {
    }

    /**
     * @brief Définit si l'opération mesurée a réussi
     */
    void setSuccess(bool success) {
        success_ = success;
    }

    ~BenchmarkGuardSuccess() override {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds"
                  << std::endl;
        if (csv_) {
            *csv_ << name_ << "," << std::fixed << std::setprecision(3) << milliseconds << "," << (success_ ? "1" : "0")
                  << std::endl;
        }
        finished_ = true;
    }

  private:
    bool success_;
};

/**
 * @brief Crée un objet BenchmarkGuardSuccess avec un nom unique basé sur le numéro de ligne
 */
#define BENCHMARK_BLOCK_CSV(name, csv_ptr) BenchmarkGuardSuccess benchmark_guard_success##__LINE__(name, csv_ptr)
