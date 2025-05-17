#ifndef BENCHMARK_HELPER_H
#define BENCHMARK_HELPER_H

/**
 * @file benchmark_helper.h
 * @brief Utilitaires pour l'exécution et la gestion des benchmarks.
 *
 * Ce fichier contient des fonctions utilitaires pour faciliter la mise en place
 * et l'exécution de benchmarks, notamment pour la génération de copies, la sauvegarde
 * des résultats et la manipulation des fichiers et répertoires nécessaires aux tests.
 */

#include <filesystem>
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <optional>
#include <functional>
#include <common.h>
#include "../external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/csv_utils.h"
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

/**
 * @brief Sauvegarde une image dans le répertoire de sortie
 * @param img Image à sauvegarder
 * @param output_dir Répertoire de sortie
 * @param output_img_path_fname Nom du fichier image
 * @param prefix Préfixe pour le nom du fichier
 */
void save_image(cv::Mat img, const std::filesystem::path& output_dir,
                const std::filesystem::path& output_img_path_fname, const std::string& prefix = "cal-");

/**
 * @brief Charge et parse le fichier JSON des AtomicBoxes
 * @param filepath Chemin du fichier JSON
 * @return JSON parsé ou lance une exception
 */
json parse_json_file(const std::string& filepath);

/**
 * @brief Structure contenant les informations sur les répertoires et le fichier CSV de benchmark
 */
struct BenchmarkSetup {
    std::filesystem::path output_dir;
    std::filesystem::path subimg_output_dir;
    std::filesystem::path csv_output_dir;
};

/**
 * @brief Prépare les répertoires et le fichier CSV pour un benchmark
 * @param output_dir Répertoire de sortie pour les résultats
 * @param include_success_column Indique si la colonne "Success" doit être incluse dans l'en-tête CSV
 * @param create_subimg_dir Indique s'il faut créer un sous-répertoire "subimg"
 * @param csv_mode Mode de gestion des fichiers CSV (append ou overwrite)
 * @return Structure BenchmarkSetup contenant les chemins et le flux CSV ouvert
 */
BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column = false,
                                             bool create_subimg_dir = false, CsvMode csv_mode = CsvMode::OVERWRITE);

#endif
