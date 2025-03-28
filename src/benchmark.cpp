#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>
#include <memory>
#include <iomanip>
#include <sstream>

#include <common.h>
#include <benchmark.hpp>
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/math_helper.h"
#include "external-tools/create_copy.h"

/**
 * @brief Sauvegarde de l'image de débogage
 *
 * @param debug_img Image de débogage
 * @param output_dir Répertoire de sortie
 * @param output_img_path_fname Chemin de l'image de sortie
 */
void save_debug_img(cv::Mat debug_img, std::filesystem::path output_dir, std::filesystem::path output_img_path_fname) {
    char* output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/cal-debug-%s", output_dir.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, debug_img);
    printf("output image: %s\n", output_img_fname);
    free(output_img_fname);
}

/**
 * @brief Crée un sous-répertoire dans le répertoire de sortie
 * @param base_dir Le répertoire de base
 * @param subdir_name Nom du sous-répertoire à créer
 * @return Chemin du sous-répertoire créé
 */
std::filesystem::path create_subdir(const std::filesystem::path& base_dir, const std::string& subdir_name) {
    std::filesystem::path subdir_path = base_dir.string() + "/" + subdir_name;
    std::filesystem::create_directories(subdir_path);
    return subdir_path;
}

/**
 * @brief Sauvegarde une image dans le répertoire de sortie
 * @param img Image à sauvegarder
 * @param output_dir Répertoire de sortie
 * @param output_img_path_fname Nom du fichier image
 * @param prefix Préfixe pour le nom du fichier
 */
void save_image(cv::Mat img, const std::filesystem::path& output_dir,
                const std::filesystem::path& output_img_path_fname, const std::string& prefix = "cal-") {
    char* output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/%s%s", output_dir.c_str(), prefix.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, img);
    printf("output image: %s\n", output_img_fname);
    free(output_img_fname);
}

/**
 * @brief Charge et parse le fichier JSON des AtomicBoxes
 * @param filepath Chemin du fichier JSON
 * @return JSON parsé ou lance une exception
 */
json parse_json_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("could not open file '" + filepath + "'");
    }

    try {
        return json::parse(file);
    } catch (const json::exception& e) {
        throw std::runtime_error("could not json parse file '" + filepath + "': " + e.what());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES DIR_IMAGE NB_COPIES\n");
        return 1;
    }

    try {
        int nb_copies = std::stoi(argv[4]);
        if (nb_copies <= 0) {
            throw std::runtime_error("Number of copies must be positive");
        }

        std::filesystem::path copies_dir = "copies";
        if (std::filesystem::exists(copies_dir)) {
            std::cout << "Cleaning existing copies directory..." << std::endl;
            std::filesystem::remove_all(copies_dir);
        }
        std::filesystem::create_directories(copies_dir);

        std::cout << "Generating " << nb_copies << " copies..." << std::endl;
        for (int i = 1; i <= nb_copies; i++) {
            std::ostringstream copy_name;
            copy_name << "copy" << std::setw(2) << std::setfill('0') << i;

            bool success = create_copy(20,               // encoded_marker_size
                                       10,               // fiducial_marker_size
                                       1,                // stroke_width
                                       5,                // marker_margin
                                       1,                // nb_copies per call
                                       0,                // duplex_printing
                                       ARUCO_WITH_QR_BR, // marker_config
                                       0,                // grey_level
                                       0,                // header_marker
                                       copy_name.str()   // filename
            );

            if (!success) {
                std::cerr << "Failed to generate " << copy_name.str() << std::endl;
            } else {
                std::cout << "Generated " << copy_name.str() << std::endl;
            }
        }

        // Création du répertoire de sortie pour les images calibrées et annotées
        std::filesystem::path output_dir{ argv[1] };

        // Nettoyage du répertoire de sortie s'il existe déjà
        if (std::filesystem::exists(output_dir)) {
            std::cout << "Cleaning existing output directory..." << std::endl;
            std::filesystem::remove_all(output_dir);
        }
        std::filesystem::create_directories(output_dir);
        std::filesystem::path subimg_output_dir = create_subdir(output_dir, "subimg");
        std::filesystem::path csv_output_dir = create_subdir(output_dir, "csv");
        
        std::filesystem::path benchmark_csv_path = csv_output_dir.string() + "/benchmark_results.csv";
        std::ofstream benchmark_csv(benchmark_csv_path);
        if (benchmark_csv.is_open()) {
            benchmark_csv << "File,Time(ms),Success" << std::endl;
        }

        // Lecture et parsing du fichier JSON des AtomicBoxes
        json atomic_boxes_json = parse_json_file(argv[2]);

        // Conversion des AtomicBox en structures
        auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
        std::vector<std::shared_ptr<AtomicBox>> corner_markers;
        std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;

        // Séparation des AtomicBox en marqueurs et boîtes utilisateur
        differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

        // Vérification et traitement du répertoire d'images d'entrée
        std::filesystem::path dir_path{ argv[3] };
        if (!std::filesystem::is_directory(dir_path)) {
            throw std::runtime_error("could not open directory '" + dir_path.string() + "'");
        }

        const cv::Point2f src_img_size{ 210, 297 };

        // Traitement de chaque image
        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            cv::Mat img = cv::imread(entry.path(), cv::IMREAD_GRAYSCALE);
            const cv::Point2f dst_img_size(img.cols, img.rows);

            auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

            Metadata meta = {
                0,
                1,
                "",
            };
#ifdef DEBUG
            cv::Mat debug_img;
            cv::cvtColor(img, debug_img, cv::COLOR_GRAY2BGR);
#endif

            std::filesystem::path output_img_path_fname = entry.path().filename().replace_extension(".png");

            std::optional<cv::Mat> affine_transform;
            BenchmarkGuardCSV benchmark_guard(entry.path().filename().string(), &benchmark_csv);

            {
                affine_transform = run_parser("aruco", img,
#ifdef DEBUG
                                              debug_img,
#endif
                                              meta, dst_corner_points);
                benchmark_guard.setSuccess(affine_transform.has_value());
            }

            // Si la transformation affine n'a pas été trouvée, on passe à l'image suivante
            if (!affine_transform.has_value()) {
#ifdef DEBUG
                save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
                fprintf(stderr, "could not find the markers\n");
                continue;
            }

            // Redressement et calibration de l'image
            auto calibrated_img_col = redress_image(img, affine_transform.value());

            cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

            // Annotation des boîtes utilisateur
            for (auto box : user_boxes_per_page[meta.page - 1]) {
                const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                           cv::Point2f{ box->x + box->width, box->y },
                                                           cv::Point2f{ box->x + box->width, box->y + box->height },
                                                           cv::Point2f{ box->x, box->y + box->height } };
                std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
                cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 255), 2);
            }

            // Annotation des marqueurs de coin
            for (auto box : corner_markers) {
                const std::vector<cv::Point2f> vec_box = { cv::Point2f{ box->x, box->y },
                                                           cv::Point2f{ box->x + box->width, box->y },
                                                           cv::Point2f{ box->x + box->width, box->y + box->height },
                                                           cv::Point2f{ box->x, box->y + box->height } };
                std::vector<cv::Point> raster_box = convert_to_raster(vec_box, src_img_size, dimension);
                cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
                cv::circle(calibrated_img_col,
                           convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } },
                                             src_img_size, dimension)[0],
                           3, cv::Scalar(0, 255, 0), -1);
            }

            // Sauvegarde de l'image calibrée et annotée
            save_image(calibrated_img_col, output_dir, output_img_path_fname);

#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
        }
        if (benchmark_csv.is_open()) {
            benchmark_csv.close();
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }

    return 0;
}
