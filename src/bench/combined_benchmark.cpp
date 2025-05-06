#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <tuple>
#include <chrono>
#include <random>

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
#include <modifier.h>

struct CopyInfo {
    std::string filename;
    double generation_time;
};

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
 * @return std::vector<cv::Point2f> Les quatre coins de la page dans l'ordre: haut-gauche, haut-droit, bas-gauche,
 * bas-droit
 */
std::vector<cv::Point2f> calculate_theoretical_corners(const cv::Point2f& src_img_size,
                                                       const cv::Point2f& dst_img_size) {
    // Les coordonnées théoriques des coins d'une page A4 en mm (0,0 est en haut à gauche)
    std::vector<cv::Point2f> theoretical_corners = {
        { 0, 0 },                          // Haut-gauche
        { src_img_size.x, 0 },             // Haut-droit
        { 0, src_img_size.y },             // Bas-gauche
        { src_img_size.x, src_img_size.y } // Bas-droit
    };

    // Mettre à l'échelle les coordonnées pour correspondre à l'image cible en pixels
    std::vector<cv::Point2f> scaled_corners;
    for (const auto& corner : theoretical_corners) {
        scaled_corners.push_back(coord_scale(corner, src_img_size, dst_img_size));
    }

    return scaled_corners;
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
                                 const cv::Mat& transform_matrix, const cv::Mat& rectification_transform,
                                 float margin) {
    std::vector<cv::Point2f> original_corners = calculate_theoretical_corners(src_img_size, dst_img_size);

    print_mat(transform_matrix);
    print_mat(rectification_transform);

    std::vector<cv::Point2f> transformed_corner = original_corners;
    cv::transform(transformed_corner, transformed_corner, transform_matrix);

    cv::transform(transformed_corner, transformed_corner, rectification_transform);

    original_corners[0].x -= margin;
    original_corners[0].y -= margin;

    original_corners[1].x += margin;
    original_corners[1].y -= margin;

    original_corners[2].x -= margin;
    original_corners[2].y += margin;

    original_corners[3].x += margin;
    original_corners[3].y += margin;

    std::vector<double> distances;
    double total_distance = 0.0;

    for (size_t i = 0; i < 4; i++) {
        double distance = euclidean_distance(original_corners[i], transformed_corner[i]);

        std::cout << "  Corner " << i << " - Original: (" << original_corners[i].x << ", " << original_corners[i].y
                  << "), after Transformation: (" << transformed_corner[i].x << ", " << transformed_corner[i].y
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

void warmup_copy(int warmup_iterations, const CopyStyleParams& style_params,
                 const CopyMarkerConfig& copy_marker_config) {
    if (warmup_iterations > 0) {
        std::cout << "Performing " << warmup_iterations << " warm-up iterations..." << std::endl;
    } else {
        std::cout << "No warm-up iterations requested." << std::endl;
        return;
    }

    for (int i = 0; i < warmup_iterations; i++) {
        std::string warmup_copy_name = "warmup" + std::to_string(i + 1);
        create_copy(style_params, copy_marker_config, warmup_copy_name, false);
        std::cout << "  Warmup iteration " << (i + 1) << "/" << warmup_iterations << " completed" << std::endl;
    }
}

std::vector<CopyInfo> bench_copy(int nb_copies, const CopyStyleParams& style_params,
                                 const CopyMarkerConfig& copy_marker_config) {
    std::vector<CopyInfo> generated_copies;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1000, 9999);

    for (int i = 1; i <= nb_copies; i++) {
        std::cout << "Generating copy " << i << "/" << nb_copies << "..." << std::endl;

        int random_suffix = dist(gen);
        std::string copy_name = "copy-" + std::to_string(i) + "-" + std::to_string(random_suffix);
        std::string filename = copy_name + ".png";

        auto create_copy_lambda = [&]() { create_copy(style_params, copy_marker_config, copy_name, false); };

        double milliseconds = Benchmark::measure("  Generation time", create_copy_lambda);

        generated_copies.push_back({ filename, milliseconds });
    }
    return generated_copies;
}

void warmup_parsing(int warmup_iterations, const std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                    const cv::Point2f& src_img_size, ParserType selected_parser,
                    const CopyMarkerConfig& copy_marker_config) {
    if (warmup_iterations > 0) {
        std::cout << "\nParsing warmup iterations..." << std::endl;
    } else {
        std::cout << "No warmup iterations requested." << std::endl;
        return;
    }
    for (int i = 0; i < warmup_iterations; i++) {
        std::string warmup_filename = "warmup" + std::to_string(i + 1) + ".png";
        std::cout << "Parsing warmup copy: " << warmup_filename << "..." << std::endl;

        cv::Mat img = cv::imread("./copies/" + warmup_filename, cv::IMREAD_GRAYSCALE);
        if (!img.data) {
            std::cerr << "Error: Could not read warmup image: " << warmup_filename << std::endl;
            continue;
        }

        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);
        Metadata meta = { 0, 1, "" };

        std::optional<cv::Mat> transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                                      cv::Mat(),
#endif
                                                      meta, dst_corner_points, copy_config_to_flag(copy_marker_config));
    }
}

void bench_parsing(std::vector<CopyInfo>& generated_copies,
                   const std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                   const cv::Point2f& src_img_size, ParserType selected_parser,
                   const CopyMarkerConfig& copy_marker_config,
                   const std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page,
                   Csv<std::string, double, double, int, std::string, CopyMarkerConfig, double>& benchmark_csv,
                   std::string output_dir) {
    for (const auto& copy_info : generated_copies) {
        std::cout << "Parsing copy: " << copy_info.filename << "..." << std::endl;

        cv::Mat img = cv::imread("./copies/" + copy_info.filename, cv::IMREAD_GRAYSCALE);
        if (!img.data) {
            std::cerr << "Error: Could not read generated image: " << copy_info.filename << std::endl;
            benchmark_csv.add_row({ copy_info.filename, copy_info.generation_time, 0, 0,
                                    parser_type_to_string(selected_parser), copy_marker_config, -1.0 });
            continue;
        }

        const cv::Point2f original_img_size(img.cols, img.rows);

        cv::Mat mat;
        random_exec(img, mat);

        std::string modified_filename = "./copies/" + copy_info.filename;
        cv::imwrite(modified_filename, img);
        std::cout << "  Saved modified image: " << modified_filename << std::endl;

#ifdef DEBUG
        cv::Mat debug_img;
        cv::cvtColor(img, debug_img, cv::COLOR_GRAY2BGR);
#endif

        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);
        Metadata meta = { 0, 1, "" };

        std::optional<cv::Mat> affine_transform;

        auto parse_lambda = [&]() {
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points, copy_config_to_flag(copy_marker_config));
        };

        double parsing_milliseconds = Benchmark::measure("  Parsing time", parse_lambda);

        bool parsing_success = affine_transform.has_value();
        std::cout << "  Success: " << (parsing_success ? "Yes" : "No") << std::endl;

        std::filesystem::path output_img_path_fname = std::filesystem::path(copy_info.filename);

#ifdef DEBUG
        save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif

        double precision_error = -1.0; // -1.0 pour indiquer une erreur
        if (parsing_success) {
            auto calibrated_img_col = redress_image(img, affine_transform.value());

            precision_error = calculate_precision_error(src_img_size, dst_img_size, mat, affine_transform.value(),
                                                        MARGIN_COPY_MODIFIED);
            std::cout << "  Precision error: " << std::fixed << std::setprecision(3) << precision_error << " pixels"
                      << std::endl;

            for (auto box : user_boxes_per_page[meta.page - 1]) {
                draw_box_outline(box, calibrated_img_col, src_img_size, dst_img_size, cv::Scalar(255, 0, 255));
            }

            for (auto box : corner_markers) {
                if (!box.has_value()) {
                    continue;
                }
                auto marker = box.value();
                draw_box_outline(marker, calibrated_img_col, src_img_size, dst_img_size, cv::Scalar(255, 0, 0));
                draw_box_center(marker, calibrated_img_col, src_img_size, dst_img_size, cv::Scalar(0, 255, 0));
            }

            save_image(calibrated_img_col, output_dir, output_img_path_fname);
        }

        // Écrire les résultats dans le CSV
        benchmark_csv.add_row({ copy_info.filename, copy_info.generation_time, parsing_milliseconds,
                                parsing_success ? 1 : 0, parser_type_to_string(selected_parser), copy_marker_config,
                                precision_error });
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

    CsvMode csv_mode = CsvMode::OVERWRITE;
    if (config.find("csv-mode") != config.end()) {
        std::string mode_str = std::get<std::string>(config.at("csv-mode").value);
        if (mode_str == "append") {
            csv_mode = CsvMode::APPEND;
            std::cout << "CSV Mode: Appending to existing CSV file if present" << std::endl;
        } else {
            csv_mode = CsvMode::OVERWRITE;
            std::cout << "CSV Mode: Overwriting existing CSV file if present" << std::endl;
        }
    }

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", true, true, csv_mode);

    Csv<std::string, double, double, int, std::string, CopyMarkerConfig, double> benchmark_csv(
        benchmark_setup.csv_output_dir / "benchmark_results.csv",
        { "File", "Generation_Time_ms", "Parsing_Time_ms", "Parsing_Success", "Parser_Type", "Copy_Config",
          "Precision_Error_px" },
        csv_mode);

    std::cout << "ÉTAPE 1: Génération des copies..." << std::endl;

    warmup_copy(warmup_iterations, style_params, copy_marker_config);

    auto generated_copies = bench_copy(nb_copies, style_params, copy_marker_config);

    /// TODO: peut-être charger un json différent par copie
    json atomic_boxes_json = parse_json_file("./original_boxes.json");
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    const cv::Point2f src_img_size{ 210, 297 };

    warmup_parsing(warmup_iterations, corner_markers, src_img_size, selected_parser, copy_marker_config);

    std::cout << "\nÉTAPE 2: Parsing des copies générées..." << std::endl;

    bench_parsing(generated_copies, corner_markers, src_img_size, selected_parser, copy_marker_config,
                  user_boxes_per_page, benchmark_csv, benchmark_setup.output_dir);

    std::cout << "Combined benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
              << " copies." << std::endl;
}