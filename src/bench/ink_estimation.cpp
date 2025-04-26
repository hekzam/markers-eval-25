#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <numeric>
#include <tuple>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"

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

        // Valeur empirique pour le facteur de calibration
        double calibration_factor = 0.001;

        return { encoded_marker_size, unencoded_marker_size, header_marker_size, grey_level, dpi,
                 copy_marker_config,  calibration_factor };
    } catch (const std::out_of_range& e) {
        throw std::invalid_argument("Missing required parameter in configuration");
    } catch (const std::bad_variant_access& e) {
        throw std::invalid_argument("Invalid parameter type in configuration");
    }
}

/**
 * @brief Benchmark d'estimation de consommation d'encre pour une seule image
 */
void ink_estimation_benchmark(const std::unordered_map<std::string, Config>& config) {
    auto [encoded_marker_size, unencoded_marker_size, header_marker_size, grey_level, dpi, copy_marker_config,
          calibration_factor] = validate_parameters(config);

    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.unencoded_marker_size = unencoded_marker_size;
    style_params.header_marker_size = header_marker_size;
    style_params.grey_level = grey_level;
    style_params.dpi = dpi;
    style_params.generating_content = false;

    std::filesystem::path dir_path{ "./copies" };
    std::filesystem::remove_all(dir_path);
    create_copy(style_params, copy_marker_config);

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

        if (!found_image) {
            throw std::runtime_error("No image file found in directory: ./copies");
        }

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
        std::cout << "Total pixel count: " << img.cols * img.rows << std::endl;
        std::cout << "Ink coverage units: " << ink_units << " pixel-units" << std::endl;
        std::cout << "Average coverage: " << (ink_units / (img.cols * img.rows)) * 100.0 << "%" << std::endl;
        std::cout << "Pixel area: " << pixel_area_cm2 << " cm²" << std::endl;
        std::cout << "Total covered area: " << total_covered_area_cm2 << " cm²" << std::endl;
        std::cout << "Estimated ink volume: " << ink_volume_ml << " ml" << std::endl;
        std::cout << "Calibration factor: " << calibration_factor << " ml/cm² at 100% coverage" << std::endl;
    }
}