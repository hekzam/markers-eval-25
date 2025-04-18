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
#include "utils/math_utils.h"
#include "utils/benchmark_helper.h"
#include "utils/draw_helper.h"
#include "external-tools/create_copy.h"
#include "command-line-interface/command_line_interface.h"

/**
 * @brief Formats d'image supportés
 */
enum class ImageFormat {
    A4,    // 210x297 mm
    A3,    // 297x420 mm
    CUSTOM // Format personnalisé
};

/**
 * @brief Obtient les dimensions d'un format d'image
 *
 * @param format Le format d'image
 * @return cv::Point2f Dimensions (largeur, hauteur) en millimètres
 */
cv::Point2f get_image_dimensions(ImageFormat format) {
    switch (format) {
        case ImageFormat::A4:
            return cv::Point2f{ 210, 297 };
        case ImageFormat::A3:
            return cv::Point2f{ 297, 420 };
        case ImageFormat::CUSTOM:
            // À implémenter si nécessaire
            return cv::Point2f{ 210, 297 };
        default:
            return cv::Point2f{ 210, 297 };
    }
}

/**
 * @brief Convertit une chaîne en format d'image
 *
 * @param format_str Chaîne de caractères représentant le format
 * @return ImageFormat Format d'image correspondant
 */
ImageFormat string_to_image_format(const std::string& format_str) {
    if (format_str == "A3")
        return ImageFormat::A3;
    if (format_str == "CUSTOM")
        return ImageFormat::CUSTOM;
    return ImageFormat::A4; // Par défaut
}

/**
 * @brief Convertit un format d'image en chaîne
 *
 * @param format Format d'image
 * @return std::string Chaîne représentant le format
 */
std::string image_format_to_string(ImageFormat format) {
    switch (format) {
        case ImageFormat::A3:
            return "A3";
        case ImageFormat::CUSTOM:
            return "CUSTOM";
        default:
            return "A4";
    }
}

/**
 * @brief Structure pour stocker les paramètres de benchmark
 */
struct BenchmarkParams {
    std::string output_dir = "./output";
    std::string atomic_boxes_file = "./original_boxes.json";
    std::string input_dir = "./copies";
    std::string copies_str = "1";
    std::string csv_filename = "benchmark_results.csv";
    int marker_config = ARUCO_WITH_QR_BR;
    ImageFormat image_format = ImageFormat::A4;
    CopyStyleParams style_params = CopyStyleParams(15, 10, 7, 2, 5, 0);
    int warmup_iterations = 2;
};

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
            return to_string(ParserType::QRCODE);
        case CIRCLES_WITH_QR_BR:
        case TOP_CIRCLES_QR_BR:
        case CIRCLE_OUTLINES_WITH_QR_BR:
            return to_string(ParserType::CIRCLE);
        case CUSTOM_SVG_WITH_QR_BR:
            return to_string(ParserType::CUSTOM_MARKER);
        case ARUCO_WITH_QR_BR:
        case TWO_ARUCO_WITH_QR_BR:
            return to_string(ParserType::ARUCO);
        case SQUARES_WITH_QR_BR:
        case SQUARE_OUTLINES_WITH_QR_BR:
        default:
            return to_string(ParserType::DEFAULT);
    }
}

/**
 * @brief Affiche la configuration actuelle du benchmark
 *
 * @param params Structure contenant les paramètres de benchmark
 */
void display_configuration(const BenchmarkParams& params) {
    std::vector<std::pair<std::string, std::string>> config_pairs = {
        { "Output directory", params.output_dir },
        { "Atomic boxes file", params.atomic_boxes_file },
        { "Input directory", params.input_dir },
        { "Copies", params.copies_str },
        { "CSV output filename", params.csv_filename },
        { "Image format", image_format_to_string(params.image_format) },
        { "Encoded marker size", std::to_string(params.style_params.encoded_marker_size) },
        { "Fiducial marker size", std::to_string(params.style_params.fiducial_marker_size) },
        { "Grey level", std::to_string(params.style_params.grey_level) },
        { "Marker config", std::to_string(params.marker_config) },
        { "Warmup iterations", std::to_string(params.warmup_iterations) }
    };

    display_configuration_recap("Configuration:", config_pairs);
}

/**
 * @brief Récupère les paramètres de benchmark à partir de l'entrée utilisateur
 *
 * @return BenchmarkParams Structure contenant les paramètres de benchmark
 */
BenchmarkParams get_benchmark_params() {
    BenchmarkParams params;
    params.output_dir = get_user_input("Output directory", params.output_dir);
    params.atomic_boxes_file = get_user_input("Atomic boxes JSON file path", params.atomic_boxes_file);
    params.input_dir = get_user_input("Input directory", params.input_dir);
    params.copies_str = get_user_input("Number of copies", params.copies_str);
    params.csv_filename = get_user_input("CSV output filename", params.csv_filename);

    std::string format_str =
        get_user_input("Image format (A4, A3, CUSTOM)", image_format_to_string(params.image_format));
    params.image_format = string_to_image_format(format_str);

    const int min_marker_size = 5, max_marker_size = 50;
    params.style_params.encoded_marker_size = get_user_input(
        "Encoded marker size", params.style_params.encoded_marker_size, &min_marker_size, &max_marker_size);
    params.style_params.fiducial_marker_size = get_user_input(
        "Fiducial marker size", params.style_params.fiducial_marker_size, &min_marker_size, &max_marker_size);

    const int min_grey = 0, max_grey = 255;
    params.style_params.grey_level = get_user_input("Grey level", params.style_params.grey_level, &min_grey, &max_grey);

    int marker_config_default =
        display_marker_configs(marker_configs, ARUCO_WITH_QR_BR, "Available marker configurations:");
    const int min_config = 1, max_config = 10;
    params.marker_config =
        get_user_input("Marker configuration (1-10)", marker_config_default, &min_config, &max_config);
        
    const int min_warmup = 0, max_warmup = 50;
    params.warmup_iterations = 
        get_user_input("Warmup iterations", params.warmup_iterations, &min_warmup, &max_warmup);

    return params;
}

/**
 * @brief Dessine des annotations pour les boîtes sur l'image
 *
 * @param image L'image sur laquelle dessiner
 * @param boxes La collection de boîtes à annoter
 * @param src_img_size Dimensions de l'image source
 * @param dst_img_size Dimensions de l'image de destination
 * @param color La couleur des annotations
 * @param draw_center Si vrai, dessine un cercle au centre de chaque boîte
 */
void annotate_boxes(cv::Mat& image, const std::vector<std::shared_ptr<AtomicBox>>& boxes,
                    const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size, const cv::Scalar& color,
                    bool draw_center = false) {
    for (auto box : boxes) {
        std::vector<cv::Point> raster_box = convert_to_raster(
            { cv::Point2f{ box->x, box->y }, cv::Point2f{ box->x + box->width, box->y },
              cv::Point2f{ box->x + box->width, box->y + box->height }, cv::Point2f{ box->x, box->y + box->height } },
            src_img_size, dst_img_size);
        cv::polylines(image, raster_box, true, color, 2);

        if (draw_center) {
            cv::circle(image,
                       convert_to_raster({ cv::Point2f{ box->x + box->width / 2, box->y + box->height / 2 } },
                                         src_img_size, dst_img_size)[0],
                       3, cv::Scalar(0, 255, 0), -1);
        }
    }
}

/**
 * @brief Structure pour stocker le contexte du benchmark
 */
struct BenchmarkContext {
    std::filesystem::path output_dir;
    std::filesystem::path subimg_output_dir;
    std::filesystem::path csv_output_dir;
    std::ofstream benchmark_csv;
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    cv::Point2f src_img_size;
};

/**
 * @brief Prépare le contexte pour l'exécution du benchmark
 *
 * @param params Structure contenant les paramètres de benchmark
 * @return BenchmarkContext Structure contenant le contexte préparé pour le benchmark
 */
BenchmarkContext prepare_benchmark(const BenchmarkParams& params) {
    BenchmarkContext context;

    // Création et nettoyage des répertoires de sortie
    context.output_dir = std::filesystem::path{ params.output_dir };
    if (std::filesystem::exists(context.output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(context.output_dir);
    }
    std::filesystem::create_directories(context.output_dir);
    context.subimg_output_dir = context.output_dir / "subimg";
    context.csv_output_dir = context.output_dir / "csv";
    std::filesystem::create_directories(context.subimg_output_dir);
    std::filesystem::create_directories(context.csv_output_dir);

    std::filesystem::path benchmark_csv_path = context.csv_output_dir / params.csv_filename;
    context.benchmark_csv.open(benchmark_csv_path);
    if (context.benchmark_csv.is_open()) {
        context.benchmark_csv << "File,Time(ms),Success" << std::endl;
    }

    if (!std::filesystem::exists(params.atomic_boxes_file)) {
        throw std::runtime_error("Atomic boxes file '" + params.atomic_boxes_file + "' does not exist");
    }

    // Lecture et parsing du JSON
    json atomic_boxes_json = parse_json_file(params.atomic_boxes_file);
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    differentiate_atomic_boxes(atomic_boxes, context.corner_markers, context.user_boxes_per_page);

    context.src_img_size = get_image_dimensions(params.image_format);

    return context;
}

/**
 * @brief Exécute le benchmark en fonction des paramètres fournis
 *
 * @param params Structure contenant les paramètres de benchmark
 * @param selected_parser Le type de parseur sélectionné
 */
void run_benchmark(const BenchmarkParams& params, const std::string& selected_parser) {
    int nb_copies = std::stoi(params.copies_str);
    if (nb_copies <= 0 || nb_copies > 50) {
        throw std::runtime_error("Number of copies must be between 1 and 50");
    }

    generate_copies(nb_copies, static_cast<MarkerConfig>(params.marker_config), params.style_params.encoded_marker_size,
                    params.style_params.fiducial_marker_size, params.style_params.grey_level);

    BenchmarkContext context = prepare_benchmark(params);

    std::filesystem::path dir_path{ params.input_dir };
    if (!std::filesystem::is_directory(dir_path)) {
        throw std::runtime_error("could not open directory '" + dir_path.string() + "'");
    }
    
    // Collecter tous les fichiers dans un vecteur
    std::vector<std::filesystem::directory_entry> all_entries;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        all_entries.push_back(entry);
    }
    
    // Vérifier si le nombre d'itérations de warmup est valide
    int warmup_iterations = params.warmup_iterations;
    if (warmup_iterations >= all_entries.size()) {
        std::cout << "Warning: Warmup iterations (" << warmup_iterations << ") is greater than or equal to the number of files (" 
                  << all_entries.size() << ")." << std::endl;
        std::cout << "Reducing warmup iterations to " << (all_entries.size() - 1) << " to ensure at least one result is recorded." << std::endl;
        warmup_iterations = (all_entries.size() > 1) ? all_entries.size() - 1 : 0;
    }
    
    // Afficher les informations sur le warmup
    if (warmup_iterations > 0) {
        std::cout << "Performing " << warmup_iterations << " warmup iterations to stabilize cache..." << std::endl;
    }

    int current_iteration = 0;
    
    for (const auto& entry : all_entries) {
        cv::Mat img = cv::imread(entry.path(), cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(context.corner_markers, context.src_img_size, dst_img_size);

        Metadata meta = { 0, 1, "" };
#ifdef DEBUG
        cv::Mat debug_img;
        cv::cvtColor(img, debug_img, cv::COLOR_GRAY2BGR);
#endif
        std::filesystem::path output_img_path_fname = entry.path().filename().replace_extension(".png");
        std::optional<cv::Mat> affine_transform;
        
        // Déterminer si cette itération est une itération de warmup
        bool is_warmup = current_iteration < warmup_iterations;
        
        if (is_warmup) {
            std::cout << "Warmup iteration " << (current_iteration + 1) << "/" << warmup_iterations 
                      << " with file: " << entry.path().filename().string() << std::endl;
                      
            // Pour les itérations de warmup, on exécute l'analyse sans enregistrer dans le CSV
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            
            // On peut toujours écrire la sortie dans le terminal pour informer l'utilisateur
            std::cout << "  Result: " << (affine_transform.has_value() ? "Success" : "Failed") << std::endl;
        } else {
            // Pour les itérations normales, on utilise le BenchmarkGuardCSV pour enregistrer dans le CSV
            BenchmarkGuardCSV benchmark_guard(entry.path().filename().string(), &context.benchmark_csv);
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            benchmark_guard.setSuccess(affine_transform.has_value());
        }
        
        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, context.output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not find the markers\n");
            current_iteration++;
            continue;
        }

        auto calibrated_img_col = redress_image(img, affine_transform.value());
        cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

        // Annotation des boîtes utilisateur et marqueurs de coin
        annotate_boxes(calibrated_img_col, context.user_boxes_per_page[meta.page - 1], context.src_img_size, dimension,
                       cv::Scalar(255, 0, 255));
        annotate_boxes(calibrated_img_col, context.corner_markers, context.src_img_size, dimension,
                       cv::Scalar(255, 0, 0), true);

        save_image(calibrated_img_col, context.output_dir, output_img_path_fname);
#ifdef DEBUG
        save_debug_img(debug_img, context.output_dir, output_img_path_fname);
#endif

        current_iteration++;
    }
    
    if (context.benchmark_csv.is_open()) {
        context.benchmark_csv.close();
    }
    
    std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and " 
              << (all_entries.size() - warmup_iterations) << " measured iterations." << std::endl;
}

int main(int argc, char* argv[]) {
    display_banner();
    BenchmarkParams params = get_benchmark_params();
    std::string selected_parser = select_parser_for_marker_config(params.marker_config);
    display_configuration(params);

    if (params.output_dir.empty() || params.atomic_boxes_file.empty() || params.input_dir.empty() ||
        params.copies_str.empty()) {
        throw std::runtime_error("Required arguments cannot be empty.");
    }

    run_benchmark(params, selected_parser);
    return 0;
}
/// TODO: load page.json