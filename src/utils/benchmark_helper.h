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
#include <optional>
#include <functional>
#include <common.h>
#include "../external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/csv_utils.h"

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
void draw_box_outline(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                      const cv::Point2f& dst_size, const cv::Scalar& color, int thickness = 2);
void draw_box_center(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                     const cv::Point2f& dst_size, const cv::Scalar& color, int radius = 3, int thickness = -1);
std::vector<double> calculate_precision_error(const cv::Point2f& dst_img_size, const cv::Mat& transform_matrix,
                                              const cv::Mat& rectification_transform, float margin);

#endif
