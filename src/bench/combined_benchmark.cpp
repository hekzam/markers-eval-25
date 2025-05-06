#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <tuple>
#include <chrono>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/math_utils.h"
#include "utils/draw_helper.h"
#include "benchmark.hpp"
#include "combined_benchmark.h"

/**
 * @brief Dessine un contour autour d'une boîte
 *
 * @param box La boîte atomique à dessiner
 * @param image L'image sur laquelle dessiner
 * @param src_size Dimensions de l'image source
 * @param dst_size Dimensions de l'image de destination
 * @param color Couleur du contour
 * @param thickness Épaisseur de la ligne
 */
void draw_box_outline(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                      const cv::Point2f& dst_size, const cv::Scalar& color, int thickness = 2) {
    std::vector<cv::Point> raster_box = convert_to_raster(
        { cv::Point2f{ box->x, box->y }, cv::Point2f{ box->x + box->width, box->y },
          cv::Point2f{ box->x + box->width, box->y + box->height }, cv::Point2f{ box->x, box->y + box->height } },
        src_size, dst_size);
    cv::polylines(image, raster_box, true, color, thickness);
}

/**
 * @brief Dessine un cercle au centre d'une boîte
 *
 * @param box La boîte atomique
 * @param image L'image sur laquelle dessiner
 * @param src_size Dimensions de l'image source
 * @param dst_size Dimensions de l'image de destination
 * @param color Couleur du cercle
 * @param radius Rayon du cercle
 * @param thickness Épaisseur du cercle (-1 pour rempli)
 */
void draw_box_center(const std::shared_ptr<AtomicBox>& box, cv::Mat& image, const cv::Point2f& src_size,
                     const cv::Point2f& dst_size, const cv::Scalar& color, int radius = 3, int thickness = -1) {
    cv::Point center =
        convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } }, src_size, dst_size)[0];
    cv::circle(image, center, radius, color, thickness);
}

/**
 * @brief Calcule la distance euclidienne entre deux points
 *
 * @param p1 Premier point
 * @param p2 Second point
 * @return double Distance euclidienne entre les deux points
 */
double euclidean_distance(const cv::Point2f& p1, const cv::Point2f& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

/**
 * @brief Calcule les coordonnées des coins théoriques d'une page A4
 * 
 * @param src_img_size Dimensions théoriques de l'image source (210x297 mm)
 * @param dst_img_size Dimensions de l'image cible en pixels
 * @return std::vector<cv::Point2f> Les quatre coins de la page dans l'ordre: haut-gauche, haut-droit, bas-gauche, bas-droit
 */
std::vector<cv::Point2f> calculate_theoretical_corners(const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size) {
    // Les coordonnées théoriques des coins d'une page A4 en mm (0,0 est en haut à gauche)
    std::vector<cv::Point2f> theoretical_corners = {
        {0, 0},                         // Haut-gauche
        {src_img_size.x, 0},            // Haut-droit
        {0, src_img_size.y},            // Bas-gauche
        {src_img_size.x, src_img_size.y} // Bas-droit
    };
    
    // Mettre à l'échelle les coordonnées pour correspondre à l'image cible en pixels
    std::vector<cv::Point2f> scaled_corners;
    for (const auto& corner : theoretical_corners) {
        scaled_corners.push_back(coord_scale(corner, src_img_size, dst_img_size));
    }
    
    return scaled_corners;
}

/**
 * @brief Calcule la matrice de rotation pour une image
 *
 * @param img_size Dimensions de l'image à transformer
 * @param angle_percent Pourcentage de rotation (1% = rotation légère)
 * @return cv::Mat Matrice de transformation
 */
cv::Mat apply_rotation_transform(const cv::Size& img_size, double angle_percent) {
    // Convertir le pourcentage en angle (1% = environ 3.6 degrés)
    double angle = angle_percent * 3.6;

    // Obtenir la matrice de rotation pour le centre de l'image
    cv::Point2f center(img_size.width / 2.0f, img_size.height / 2.0f);
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, angle, 1.0);

    return rotation_matrix;
}

/**
 * @brief Calcule l'erreur moyenne de précision entre les coins originaux et les coins calibrés
 * 
 * @param src_img_size Dimensions théoriques de l'image source (210x297 mm)
 * @param dst_img_size Dimensions de l'image calibrée en pixels
 * @param rotation_angle_percent Pourcentage de rotation appliqué à l'image
 * @param affine_transform Matrice de transformation affine du parser
 * @return double Erreur moyenne en pixels
 */
double calculate_precision_error(const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size,
                               double rotation_angle_percent, const cv::Mat& affine_transform) {
    // 1. Calculer les coordonnées théoriques des quatre coins
    std::vector<cv::Point2f> original_corners = calculate_theoretical_corners(src_img_size, dst_img_size);
    
    // 2. Utiliser la même fonction apply_rotation_transform pour générer la matrice de rotation
    cv::Size img_size(dst_img_size.x, dst_img_size.y);
    auto rotation_matrix = apply_rotation_transform(img_size, rotation_angle_percent);
    
    // 3. Appliquer la rotation directement aux coins originaux
    std::vector<cv::Point2f> rotated_corners = original_corners;
    cv::transform(rotated_corners, rotated_corners, rotation_matrix);
    
    // 4. Appliquer la transformation affine inverse aux coins originaux
    cv::Mat inverse_affine;
    cv::invertAffineTransform(affine_transform, inverse_affine);
    
    std::vector<cv::Point2f> expected_corners = original_corners;
    cv::transform(expected_corners, expected_corners, inverse_affine);
    
    // 5. Calculer la distance entre les coins après rotation et ceux attendus par le parseur
    std::vector<double> distances;
    double total_distance = 0.0;
    
    for (size_t i = 0; i < 4; i++) {
        double distance = euclidean_distance(rotated_corners[i], expected_corners[i]);
        
        std::cout << "  Corner " << i << " - Rotated: (" << rotated_corners[i].x << ", " << rotated_corners[i].y 
                  << "), Parser expected: (" << expected_corners[i].x << ", " << expected_corners[i].y 
                  << "), Precision error: " << distance << " pixels" << std::endl;
        
        total_distance += distance;
        distances.push_back(distance);
    }
    
    // Calculer l'erreur moyenne
    if (!distances.empty()) {
        double avg_error = total_distance / distances.size();
        std::cout << "  Average precision error: " << avg_error << " pixels" << std::endl;
        return avg_error;
    }
    
    return -1.0; // Valeur d'erreur en cas d'absence de coins
}

/**
 * @brief Applique une rotation à l'image en utilisant la matrice de rotation calculée
 *
 * @param img Image à transformer
 * @param rotation_matrix Matrice de rotation à appliquer
 * @param margin Taille de la marge (non utilisée, conservée pour compatibilité API)
 * @return cv::Mat Image transformée avec le même ratio que l'original
 */
cv::Mat apply_rotation_to_image(const cv::Mat& img, const cv::Mat& rotation_matrix, int margin) {
    // Dimensions de l'image originale
    cv::Size img_size = img.size();

    // Appliquer la transformation
    cv::Mat rotated_img;
    cv::warpAffine(img, rotated_img, rotation_matrix, img_size, cv::INTER_LINEAR,
                   cv::BORDER_CONSTANT, cv::Scalar(255));

    return rotated_img;
}

/**
 * @brief Vérifie et extrait les paramètres de configuration
 * @param config Map de configuration contenant les paramètres
 * @return Tuple contenant les paramètres validés
 * @throws std::invalid_argument Si un paramètre requis est manquant ou invalide
 */
static std::tuple<int, int, int, int, int, int, int, CopyMarkerConfig, ParserType>
validate_parameters_combined(const std::unordered_map<std::string, Config>& config) {
    try {
        int warmup_iterations = std::get<int>(config.at("warmup-iterations").value);
        int nb_copies = std::get<int>(config.at("nb-copies").value);
        int encoded_marker_size = std::get<int>(config.at("encoded-marker-size").value);
        int unencoded_marker_size = std::get<int>(config.at("unencoded-marker-size").value);
        int header_marker_size = std::get<int>(config.at("header-marker-size").value);
        int grey_level = std::get<int>(config.at("grey-level").value);
        int dpi = std::get<int>(config.at("dpi").value);
        auto marker_config = std::get<std::string>(config.at("marker-config").value);

        CopyMarkerConfig copy_marker_config;
        if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
            throw std::invalid_argument("Invalid marker configuration: " + marker_config);
        }

        // Permettre à l'utilisateur de spécifier explicitement le parseur
        ParserType selected_parser = ParserType::QRCODE; // Parseur par défaut: QRCODE

        // Si le parseur est spécifié dans la configuration, l'utiliser
        if (config.find("parser-type") != config.end()) {
            std::string parser_type_str = std::get<std::string>(config.at("parser-type").value);
            selected_parser = string_to_parser_type(parser_type_str);

            if (parser_type_str != parser_type_to_string(selected_parser)) {
                std::cout << "Warning: Unknown parser type '" << parser_type_str << "', using default QRCODE parser."
                          << std::endl;
            }
        } else {
            std::cout << "Note: No parser type specified, using default QRCODE parser." << std::endl;
        }

        return { warmup_iterations, nb_copies, encoded_marker_size, unencoded_marker_size, header_marker_size,
                 grey_level,        dpi,       copy_marker_config,  selected_parser };
    } catch (const std::out_of_range& e) {
        throw std::invalid_argument("Missing required parameter in configuration");
    } catch (const std::bad_variant_access& e) {
        throw std::invalid_argument("Invalid parameter type in configuration");
    }
}

void combined_benchmark(const std::unordered_map<std::string, Config>& config) {
    auto [warmup_iterations, nb_copies, encoded_marker_size, unencoded_marker_size, header_marker_size, grey_level, dpi,
          copy_marker_config, selected_parser] = validate_parameters_combined(config);

    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.unencoded_marker_size = unencoded_marker_size;
    style_params.header_marker_size = header_marker_size;
    style_params.grey_level = grey_level;
    style_params.dpi = dpi;

    // Préparation des répertoires et du fichier CSV
    BenchmarkSetup benchmark_setup = prepare_benchmark_directories(
        "./output", true, true, false); // Le dernier paramètre est mis à false pour ne pas écrire l'en-tête par défaut
    std::ofstream& benchmark_csv = benchmark_setup.benchmark_csv;

    // Ajouter l'en-tête du CSV avec les colonnes demandées
    benchmark_csv << "File,Generation_Time_ms,Parsing_Time_ms,Parsing_Success,Parser_Type,Precision_Error_px"
                  << std::endl;

    // Structure pour stocker les informations de génération des copies
    struct CopyInfo {
        std::string filename;
        double generation_time;
    };

    std::vector<CopyInfo> generated_copies;

    // ÉTAPE 1: Générer toutes les copies et enregistrer le temps de génération
    std::cout << "ÉTAPE 1: Génération des copies..." << std::endl;

    // Nettoyer le répertoire des copies avant de commencer
    std::filesystem::path copies_dir = "./copies";
    if (std::filesystem::exists(copies_dir)) {
        std::cout << "Cleaning existing copies directory..." << std::endl;
        std::filesystem::remove_all(copies_dir);
    }
    std::filesystem::create_directories(copies_dir);

    // Réaliser le warmup si nécessaire
    if (warmup_iterations > 0) {
        std::cout << "Performing " << warmup_iterations << " warm-up iterations..." << std::endl;
        for (int i = 0; i < warmup_iterations; i++) {
            std::string warmup_copy_name = "warmup" + std::to_string(i + 1);
            create_copy(style_params, copy_marker_config, warmup_copy_name, false);
            std::cout << "  Warmup iteration " << (i + 1) << "/" << warmup_iterations << " completed" << std::endl;
        }
    }

    // Générer toutes les copies et mesurer le temps
    for (int i = 1; i <= nb_copies; i++) {
        std::cout << "Generating copy " << i << "/" << nb_copies << "..." << std::endl;

        std::string copy_name = "copy" + std::to_string(i);
        std::string filename = copy_name + ".png";

        // Créer une lambda pour la génération de copie
        auto create_copy_lambda = [&]() { create_copy(style_params, copy_marker_config, copy_name, false); };

        // Utiliser directement Benchmark::measure qui retourne maintenant le temps mesuré
        double milliseconds = Benchmark::measure("  Generation time", create_copy_lambda);

        // Stocker les informations de la copie pour l'étape de parsing
        generated_copies.push_back({ filename, milliseconds });
    }

    // Charger une seule fois les boîtes atomiques pour le parsing
    json atomic_boxes_json = parse_json_file("./original_boxes.json");
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    const cv::Point2f src_img_size{ 210, 297 };

    // Effectuer le warmup pour le parsing si des itérations de chauffe ont été utilisées
    if (warmup_iterations > 0) {
        std::cout << "\nParsing warmup iterations..." << std::endl;
        for (int i = 0; i < warmup_iterations; i++) {
            std::string warmup_filename = "warmup" + std::to_string(i + 1) + ".png";
            std::cout << "Parsing warmup copy: " << warmup_filename << "..." << std::endl;

            // Charger l'image de la copie de warmup
            cv::Mat img = cv::imread("./copies/" + warmup_filename, cv::IMREAD_GRAYSCALE);
            if (!img.data) {
                std::cerr << "Error: Could not read warmup image: " << warmup_filename << std::endl;
                continue;
            }

            const cv::Point2f dst_img_size(img.cols, img.rows);
            auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);
            Metadata meta = { 0, 1, "" };

            // Créer une lambda pour le parsing de warmup
            auto warmup_parse_lambda = [&]() {
                std::optional<cv::Mat> transform =
                    run_parser(selected_parser, img, meta, dst_corner_points, copy_config_to_flag(copy_marker_config));
            };

            // Mesurer le temps mais ne pas enregistrer les résultats dans le CSV
            Benchmark::measure("  Warmup parsing time", warmup_parse_lambda);
        }
    }

    // ÉTAPE 2: Parser les copies générées et compléter le CSV
    std::cout << "\nÉTAPE 2: Parsing des copies générées..." << std::endl;

    // Parser chaque copie générée
    for (const auto& copy_info : generated_copies) {
        std::cout << "Parsing copy: " << copy_info.filename << "..." << std::endl;

        // Charger l'image de la copie
        cv::Mat img = cv::imread("./copies/" + copy_info.filename, cv::IMREAD_GRAYSCALE);
        if (!img.data) {
            std::cerr << "Error: Could not read generated image: " << copy_info.filename << std::endl;
            benchmark_csv << copy_info.filename << "," << copy_info.generation_time << ",0,0" << std::endl;
            continue;
        }

        // Appliquer une rotation de 1% à l'image avant parsing
        std::cout << "  Applying 1% rotation transformation before parsing..." << std::endl;
        auto rotation_matrix = apply_rotation_transform(img.size(), 0);
        img = apply_rotation_to_image(img, rotation_matrix, 0);  // Paramètre margin mis à 0

        // Sauvegarder l'image avec la rotation appliquée dans le dossier des copies
        std::string rotated_filename = "./copies/" + copy_info.filename;
        cv::imwrite(rotated_filename, img);
        std::cout << "  Saved rotated image: " << rotated_filename << std::endl;

        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);
        Metadata meta = { 0, 1, "" };

        // Variables pour stocker les résultats
        std::optional<cv::Mat> affine_transform;
        bool parsing_success = false;

        // Créer une lambda pour le parsing
        auto parse_lambda = [&]() {
            affine_transform =
                run_parser(selected_parser, img, meta, dst_corner_points, copy_config_to_flag(copy_marker_config));
        };

        // Utiliser directement Benchmark::measure qui retourne maintenant le temps mesuré
        double parsing_milliseconds = Benchmark::measure("  Parsing time", parse_lambda);

        // Vérifier le succès du parsing
        parsing_success = affine_transform.has_value();
        std::cout << "  Success: " << (parsing_success ? "Yes" : "No") << std::endl;

        // Calculer l'erreur de précision
        double precision_error = -1.0;
        if (parsing_success) {
            // Si le parsing a réussi, sauvegarder l'image redressée avec les marqueurs
            if (parsing_success) {
                auto calibrated_img_col = redress_image(img, affine_transform.value());
                cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

                // Calculer l'erreur de précision sur l'image redressée
                precision_error = calculate_precision_error(src_img_size, dimension, 0, affine_transform.value());
                std::cout << "  Precision error: " << std::fixed << std::setprecision(3) << precision_error << " pixels" << std::endl;

                // Dessiner les boîtes utilisateur
                for (auto box : user_boxes_per_page[meta.page - 1]) {
                    draw_box_outline(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 255));
                }

                // Dessiner les marqueurs de coin
                for (auto box : corner_markers) {
                    if (!box.has_value()) {
                        continue;
                    }
                    auto marker = box.value();
                    draw_box_outline(marker, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 0));
                    draw_box_center(marker, calibrated_img_col, src_img_size, dimension, cv::Scalar(0, 255, 0));
                }

                // Sauvegarder l'image redressée
                std::filesystem::path output_img_path_fname = std::filesystem::path(copy_info.filename);
                // Ne pas ajouter l'extension .png car le fichier a déjà une extension
                save_image(calibrated_img_col, benchmark_setup.output_dir, output_img_path_fname);
            }
        }

        // Écrire les résultats dans le CSV
        benchmark_csv << copy_info.filename << "," << copy_info.generation_time << "," << parsing_milliseconds << ","
                      << (parsing_success ? "1" : "0") << "," << parser_type_to_string(selected_parser) << ","
                      << precision_error << std::endl;
    }

    std::cout << "Combined benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
              << " copies." << std::endl;

    // Fermer le fichier CSV
    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }
}