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
#include <string>
#include <common.h>
#include "../external-tools/create_copy.h"
#include "../utils/cli_helper.h"

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
 * @brief Crée un sous-répertoire dans le répertoire de sortie
 * @param base_dir Le répertoire de base
 * @param subdir_name Nom du sous-répertoire à créer
 * @return Chemin du sous-répertoire créé
 */
std::filesystem::path create_subdir(const std::filesystem::path& base_dir, const std::string& subdir_name);

/**
 * @brief Charge et parse le fichier JSON des AtomicBoxes
 * @param filepath Chemin du fichier JSON
 * @return JSON parsé ou lance une exception
 */
json parse_json_file(const std::string& filepath);

/**
 * @brief Génère des copies de marqueurs avec une configuration spécifique
 * @param config Configuration des copies à générer
 * @param style_params Paramètres de style pour la génération
 * @param is_benchmark Indique si on exécute un benchmark
 * @param benchmark_csv Fichier CSV pour enregistrer les résultats du benchmark
 * @return true si toutes les copies ont été générées avec succès, false sinon
 */
bool generate_copies(const std::map<std::string, Config>& config, const CopyStyleParams& style_params,
                     bool is_benchmark, std::ofstream& benchmark_csv);

/**
 * @brief Génère des copies de marqueurs sans enregistrer de benchmarks
 * @param config Configuration des copies à générer
 * @param style_params Paramètres de style pour la génération
 * @return true si toutes les copies ont été générées avec succès, false sinon
 */
bool generate_copies(const std::map<std::string, Config>& config, const CopyStyleParams& style_params);

/**
 * @brief Structure contenant les informations sur les répertoires et le fichier CSV de benchmark
 */
struct BenchmarkSetup {
    std::filesystem::path output_dir;
    std::filesystem::path subimg_output_dir;
    std::filesystem::path csv_output_dir;
    std::ofstream benchmark_csv;
};

/**
 * @brief Prépare les répertoires et le fichier CSV pour un benchmark
 * @param config La configuration contenant le chemin du répertoire de sortie
 * @param include_success_column Indique si la colonne "Success" doit être incluse dans l'en-tête CSV
 * @param create_subimg_dir Indique s'il faut créer un sous-répertoire "subimg"
 * @return Structure BenchmarkSetup contenant les chemins et le flux CSV ouvert
 */
BenchmarkSetup prepare_benchmark_directories(const std::map<std::string, Config>& config, 
                                            bool include_success_column = false, 
                                            bool create_subimg_dir = false);

#endif
