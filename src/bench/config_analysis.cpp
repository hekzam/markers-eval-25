#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <numeric>
#include <tuple>
#include <random>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/csv_utils.h"

#define INCH 2.54

/**
 * @brief Vérifie et extrait les paramètres de configuration
 * @param config Map de configuration contenant les paramètres
 * @return Tuple contenant les paramètres validés
 * @throws std::invalid_argument Si un paramètre requis est manquant ou invalide
 */
static std::tuple<int, int, int, int, int, CopyMarkerConfig, double>
validate_parameters(const std::unordered_map<std::string, Config>& config) {
    try {
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

        double calibration_factor = 0.001;
        if (config.find("calibration-factor") != config.end()) {
            int cal_factor_int = std::get<int>(config.at("calibration-factor").value);
            calibration_factor = cal_factor_int / 1000.0;
            std::cout << "Using calibration factor: " << calibration_factor << " ml/cm²" << std::endl;
        } else {
            std::cout << "No calibration factor provided, using default: " << calibration_factor << " ml/cm²" << std::endl;
        }

        return { encoded_marker_size, unencoded_marker_size, header_marker_size, grey_level, dpi,
                 copy_marker_config,  calibration_factor };
    } catch (const std::out_of_range& e) {
        throw std::invalid_argument("Missing required parameter in configuration");
    } catch (const std::bad_variant_access& e) {
        throw std::invalid_argument("Invalid parameter type in configuration");
    }
}

/**
 * @brief Analyse la consommation d'encre pour une image
 * @param image_path Chemin vers l'image à analyser
 * @param dpi Résolution de l'image en points par pouce
 * @param calibration_factor Facteur de calibration pour le calcul du volume d'encre
 * @return double Volume d'encre estimé en ml
 */
static double analyze_ink_consumption(const std::filesystem::path& image_path, int dpi, double calibration_factor) {
    cv::Mat img;
    img = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        throw std::runtime_error("Failed to load image: " + image_path.string());
    }

    std::cout << "Analyzing image: " << image_path.string() << std::endl;
    std::cout << "Image dimensions: " << img.cols << "x" << img.rows << " pixels" << std::endl;

    cv::Mat normalized;
    img.convertTo(normalized, CV_32F, 1.0 / 255.0);

    if (normalized.empty()) {
        throw std::runtime_error("Failed to normalize image");
    }

    cv::Mat coverage = 1.0 - normalized;
    double ink_units = cv::sum(coverage)[0];

    if (ink_units < 0) {
        throw std::runtime_error("Calculated negative ink coverage");
    }

    double pixel_area_cm2 = pow(INCH / dpi, 2);
    // dpi : pixels per inch
    // largeur d'un pixel en cm = 1/dpi * 2.54 cm/inch
    // aire d'un pixel en cm² = (1/dpi)² * (2.54 cm/inch)²

    double total_covered_area_cm2 = ink_units * pixel_area_cm2;
    double ink_volume_ml = total_covered_area_cm2 * calibration_factor;
    // Formule empirique pour estimer le volume d'encre en ml
    // volume d'encre en ml = aire totale couverte en cm² * facteur de calibration

    std::cout << "\n=== Ink Consumption Analysis Results ===" << std::endl;
    std::cout << "Estimated ink volume: " << ink_volume_ml << " ml" << std::endl;
    std::cout << "Calibration factor: " << calibration_factor << " ml/cm² at 100% coverage" << std::endl;

    return ink_volume_ml;
}

/**
 * @brief Calcule l'aire d'un marqueur en cm²
 * @param marker Pointeur partagé vers le marqueur
 * @return Aire du marqueur en cm²
 */
static double calculate_marker_area_cm2(const std::shared_ptr<AtomicBox>& marker) {
    if (marker == nullptr) {
        return 0.0;
    }

    double width_cm = marker->width * 0.1;
    double height_cm = marker->height * 0.1;
    if (marker->stroke_width > 0) {
        width_cm += marker->stroke_width / 2 * 0.1;
        height_cm += marker->stroke_width / 2 * 0.1;
    }

    return width_cm * height_cm;
}

/**
 * @brief Analyse l'aire des marqueurs de coin
 * @param corner_markers Vecteur des marqueurs de coin
 * @return Aire totale des marqueurs de coin en cm²
 */
static double
analyze_corner_markers_area(const std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers) {
    double total_corner_markers_area_cm2 = 0.0;
    std::cout << "\n=== Corner Markers Areas ===" << std::endl;

    const std::string corner_names[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Top Center" };

    for (size_t i = 0; i < corner_markers.size(); i++) {
        if (corner_markers[i].has_value() == false) {
            continue;
        }
        double area_cm2 = calculate_marker_area_cm2(corner_markers[i].value());
        total_corner_markers_area_cm2 += area_cm2;

        if (corner_markers[i] != nullptr) {
            std::cout << corner_names[i] << " marker area: " << area_cm2 << " cm²" << std::endl;
        }
    }

    std::cout << "Total corner markers area: " << total_corner_markers_area_cm2 << " cm²" << std::endl;
    return total_corner_markers_area_cm2;
}

/**
 * @brief Benchmark d'estimation de consommation d'encre pour une seule image
 */
void config_analysis_benchmark(const std::unordered_map<std::string, Config>& config) {
    auto [encoded_marker_size, unencoded_marker_size, header_marker_size, grey_level, dpi, copy_marker_config,
          calibration_factor] = validate_parameters(config);

    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.unencoded_marker_size = unencoded_marker_size;
    style_params.header_marker_size = header_marker_size;
    style_params.grey_level = grey_level;
    style_params.dpi = dpi;
    style_params.generating_content = false;

    // Configuration du CSV
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

    std::string csv_filename = "ink_estimation_results.csv";
    if (config.find("csv-filename") != config.end()) {
        csv_filename = std::get<std::string>(config.at("csv-filename").value);
        std::cout << "CSV Filename: " << csv_filename << std::endl;
    }

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", false, false, csv_mode);

    Csv<std::string, CopyMarkerConfig, int, double, double, double> ink_estimation_csv(
        benchmark_setup.csv_output_dir / csv_filename,
        { "File", "Copy_Config", "DPI", "Ink_Consumption_ml", "Calibration_Factor", "Total_Markers_Area_cm2" },
        csv_mode);

    std::filesystem::path dir_path{ "./copies" };
    std::filesystem::remove_all(dir_path);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1000, 9999);
    int random_suffix = dist(gen);
    
    std::string copy_name = "copy-" + std::to_string(random_suffix);
    
    create_copy(style_params, copy_marker_config, copy_name);

    std::filesystem::path image_path;
    bool found_image = false;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
                image_path = entry.path();
                found_image = true;
                break;
            }
        }
    }

    if (!found_image) {
        throw std::runtime_error("No image file found in directory: ./copies");
    }

    // Analyse de la consommation d'encre
    double ink_volume_ml = analyze_ink_consumption(image_path, dpi, calibration_factor);

    std::ifstream atomic_boxes_file("./original_boxes.json");
    if (!atomic_boxes_file.is_open()) {
        throw std::runtime_error("could not open file './original_boxes.json'");
    }

    json atomic_boxes_json;
    try {
        atomic_boxes_json = json::parse(atomic_boxes_file);
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("could not json parse file './original_boxes.json': ") + e.what());
    }

    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    double total_corner_markers_area_cm2 = analyze_corner_markers_area(corner_markers);

    // Ajout des résultats au CSV
    ink_estimation_csv.add_row(std::make_tuple(image_path.filename().string(), copy_marker_config, dpi, ink_volume_ml,
                                               calibration_factor, total_corner_markers_area_cm2));

    std::cout << "\nInk estimation benchmark completed successfully." << std::endl;
    std::cout << "Results saved to: " << (benchmark_setup.csv_output_dir / csv_filename).string() << std::endl;
}