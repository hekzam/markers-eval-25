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
#include "utils/benchmark_helper.h"
#include "utils/draw_helper.h"
#include "external-tools/create_copy.h"

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string GREEN = "\033[32m";
const std::string BLUE = "\033[34m";
const std::string CYAN = "\033[36m";
const std::string YELLOW = "\033[33m";

const std::string PARSER_ARUCO = "aruco";
const std::string PARSER_CIRCLE = "circle";
const std::string PARSER_QRCODE = "qrcode";
const std::string PARSER_CUSTOM_MARKER = "custom_marker";
const std::string PARSER_DEFAULT = "default";

/**
 * @brief Affiche une bannière de bienvenue pour le programme
 */
void display_banner() {
    std::cout << std::string(50, '-') << std::endl;
    std::cout << BOLD << BLUE << "  BENCHMARK TOOL - MARKERS EVALUATION" << RESET << std::endl;
    std::cout << CYAN << "  Document Processing & Analysis" << RESET << std::endl;
    std::cout << std::string(50, '-') << std::endl << std::endl;
}

/**
 * @brief Affiche une liste de configurations disponibles
 *
 * @param marker_configs Liste des configurations à afficher
 * @param title Titre à afficher en gras avant la liste
 * @return int La configuration par défaut à utiliser
 */
int display_configs(const std::vector<MarkerConfigInfo>& marker_configs, const int default_config,
                    const std::string& title = "Available configurations:") {

    std::cout << std::endl << BOLD << title << RESET << std::endl;

    for (const auto& config : marker_configs) {
        std::cout << std::setw(3) << config.id << ". " << config.description << std::endl;
    }

    return default_config;
}

/**
 * @brief Demande une entrée numérique à l'utilisateur avec formatage amélioré et vérification de plage
 *
 * @param prompt Message à afficher
 * @param default_value Valeur par défaut
 * @param min_value Valeur minimale autorisée
 * @param max_value Valeur maximale autorisée
 * @return int Valeur numérique entrée par l'utilisateur ou valeur par défaut
 */
int get_user_input_int(const std::string& prompt, int default_value, int min_value, int max_value) {
    std::string input;
    int value = default_value;
    bool valid_input = false;

    do {
        std::cout << BOLD << prompt << RESET << " [" << GREEN << default_value << RESET << "]: ";
        std::getline(std::cin, input);

        if (input.empty()) {
            value = default_value;
            valid_input = true;
        } else {
            try {
                value = std::stoi(input);
                if (value >= min_value && value <= max_value) {
                    valid_input = true;
                } else {
                    std::cout << "Value must be between " << min_value << " and " << max_value << ". Please try again."
                              << std::endl;
                }
            } catch (const std::exception& e) { std::cout << "Invalid input. Please enter a number." << std::endl; }
        }
    } while (!valid_input);

    return value;
}

/**
 * @brief Vérifie si une chaîne ne contient que des espaces
 *
 * @param str Chaîne à vérifier
 * @return true si la chaîne est vide ou ne contient que des espaces
 * @return false sinon
 */
bool is_whitespace_only(const std::string& str) {
    return str.empty() || str.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
}

/**
 * @brief Demande une entrée à l'utilisateur avec formatage amélioré
 *
 * @param prompt Message à afficher
 * @param default_value Valeur par défaut
 * @return std::string Valeur entrée par l'utilisateur ou valeur par défaut
 */
std::string get_user_input(const std::string& prompt, const std::string& default_value) {
    std::string input;
    std::cout << BOLD << prompt << RESET << " [" << GREEN << default_value << RESET << "]: ";
    std::getline(std::cin, input);
    return is_whitespace_only(input) ? default_value : input;
}

/**
 * @brief Sélectionne le type de parseur approprié en fonction de la configuration de marqueur choisie
 *
 * @param marker_config Le numéro de configuration de marqueur (1-10)
 * @return std::string Le type de parseur à utiliser
 */
std::string select_parser_for_marker_config(int marker_config) {
    switch (marker_config) {
        case QR_ALL_CORNERS:
        case QR_BOTTOM_RIGHT_ONLY:
            return PARSER_QRCODE;
        case CIRCLES_WITH_QR_BR:
        case TOP_CIRCLES_QR_BR:
        case CIRCLE_OUTLINES_WITH_QR_BR:
            return PARSER_CIRCLE;
        case CUSTOM_SVG_WITH_QR_BR:
            return PARSER_CUSTOM_MARKER;
        case ARUCO_WITH_QR_BR:
        case TWO_ARUCO_WITH_QR_BR:
            return PARSER_ARUCO;
        case SQUARES_WITH_QR_BR:
        case SQUARE_OUTLINES_WITH_QR_BR:
        default:
            return PARSER_DEFAULT;
    }
}

int main(int argc, char* argv[]) {
    std::string output_dir_opt = "./output";
    std::string atomic_boxes_file = "./original_boxes.json";
    std::string input_dir = "./copies";
    std::string copies_str = "1";
    int encoded_marker_size = 15;
    int fiducial_marker_size = 10;
    int grey_level = 0;

    std::string input;

    // Affichage de la bannière
    display_banner();

    // Récupération des entrées utilisateur avec un format amélioré
    output_dir_opt = get_user_input("Output directory", output_dir_opt);
    atomic_boxes_file = get_user_input("Atomic boxes JSON file path", atomic_boxes_file);
    input_dir = get_user_input("Input directory", input_dir);
    copies_str = get_user_input("Number of copies", copies_str);
    encoded_marker_size = get_user_input_int("Encoded marker size", encoded_marker_size, 5, 50);
    fiducial_marker_size = get_user_input_int("Fiducial marker size", fiducial_marker_size, 5, 50);
    grey_level = get_user_input_int("Grey level", grey_level, 0, 255);
    int marker_config_default = display_configs(marker_configs, ARUCO_WITH_QR_BR, "Available marker configurations:");
    int marker_config = get_user_input_int("Marker configuration (1-10)", marker_config_default, 1, 10);

    // Sélectionne le parseur à utiliser en fonction de la configuration choisie
    std::string selected_parser = select_parser_for_marker_config(marker_config);

    // Affichage du récapitulatif
    std::cout << std::endl << BOLD << "Configuration:" << RESET << std::endl;
    std::cout << "- Output directory: " << YELLOW << output_dir_opt << RESET << std::endl;
    std::cout << "- Atomic boxes file: " << YELLOW << atomic_boxes_file << RESET << std::endl;
    std::cout << "- Input directory: " << YELLOW << input_dir << RESET << std::endl;
    std::cout << "- Copies: " << YELLOW << copies_str << RESET << std::endl;
    std::cout << "- Encoded marker size: " << YELLOW << encoded_marker_size << RESET << std::endl;
    std::cout << "- Fiducial marker size: " << YELLOW << fiducial_marker_size << RESET << std::endl;
    std::cout << "- Grey level: " << YELLOW << grey_level << RESET << std::endl;
    std::cout << "- Marker config: " << YELLOW << marker_config << RESET << std::endl << std::endl;

    // Verify values are not empty (should never happen with defaults)
    if (output_dir_opt.empty() || atomic_boxes_file.empty() || input_dir.empty() || copies_str.empty()) {
        std::cerr << "Required arguments cannot be empty.\n";
        return 1;
    }

    int nb_copies = std::stoi(copies_str);
    if (nb_copies <= 0 || nb_copies > 50) {
        throw std::runtime_error("Number of copies must be between 1 and 50");
    }

    generate_copies(nb_copies, static_cast<MarkerConfig>(marker_config), encoded_marker_size, fiducial_marker_size,
                    grey_level);

    // Création du répertoire de sortie pour les images calibrées et annotées
    std::filesystem::path output_dir{ output_dir_opt };

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

    // Vérification de l'existence du fichier atomic_boxes
    if (!std::filesystem::exists(atomic_boxes_file)) {
        throw std::runtime_error("Atomic boxes file '" + atomic_boxes_file + "' does not exist");
    }

    // Lecture et parsing du fichier JSON des AtomicBoxes
    json atomic_boxes_json = parse_json_file(atomic_boxes_file);

    // Conversion des AtomicBox en structures
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;

    // Séparation des AtomicBox en marqueurs et boîtes utilisateur
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    // Vérification et traitement du répertoire d'images d'entrée
    std::filesystem::path dir_path{ input_dir };
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
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            benchmark_guard.setSuccess(affine_transform.has_value());
        }
        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not find the markers\n");
            continue;
        }

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

        save_image(calibrated_img_col, output_dir, output_img_path_fname);
#ifdef DEBUG
        save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
    }
    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
    return 0;
}
