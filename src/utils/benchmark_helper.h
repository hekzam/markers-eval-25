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
 * @param nb_copies Nombre de copies à générer
 * @param marker_config_id ID de la configuration des marqueurs à utiliser
 * @param style_params Paramètres de style pour la génération
 * @return true si toutes les copies ont été générées avec succès, false sinon
 */
bool generate_copies(int nb_copies, int marker_config_id, const CopyStyleParams& style_params);

#endif
