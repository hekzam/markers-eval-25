#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <numeric>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"

#define INCH 2.54

/**
 * @brief Benchmark d'estimation de consommation d'encre pour une seule image
 */
void ink_estimation_benchmark(const std::unordered_map<std::string, Config>& config) {
    try {
        auto encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
        auto unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
        auto header_marker_size = std::get<int>(config.at("header-marker_size").value);
        auto grey_level = std::get<int>(config.at("grey-level").value);
        auto dpi = std::get<int>(config.at("dpi").value);
        auto marker_config = std::get<std::string>(config.at("marker-config").value);

        CopyMarkerConfig copy_marker_config;
        if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
            std::cerr << "Invalid marker configuration: " << marker_config << std::endl;
            return;
        }

        double calibration_factor = 0.001;

        CopyStyleParams style_params;
        style_params.encoded_marker_size = encoded_marker_size;
        style_params.unencoded_marker_size = unencoded_marker_size;
        style_params.grey_level = grey_level;
        style_params.dpi = dpi;
        style_params.generating_content = false;

        std::filesystem::path dir_path{ "./copies" };
        std::filesystem::remove_all(dir_path);
        create_copy(style_params, copy_marker_config);

        std::filesystem::path image_path;
        bool found_image = false;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tif" ||
                        ext == ".tiff") {
                        image_path = entry.path();
                        found_image = true;
                        break;
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            throw std::runtime_error("Error accessing directory: " + std::string(e.what()));
        }

        if (!found_image) {
            throw std::runtime_error("No image file found in directory: ./copies");
        }

        cv::Mat img;
        try {
            img = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                throw std::runtime_error("Failed to load image: " + image_path.string());
            }
        } catch (const cv::Exception& e) {
            throw std::runtime_error("OpenCV error while loading image: " + std::string(e.what()));
        }

        std::cout << "Analyzing image: " << image_path.string() << std::endl;

        std::cout << "Image dimensions: " << img.cols << "x" << img.rows << " pixels" << std::endl;

        try {
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
            // aire totale couverte en cm² = somme des valeurs de couverture d'encre * aire d'un pixel en cm²
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
        } catch (const cv::Exception& e) {
            throw std::runtime_error("OpenCV error during image processing: " + std::string(e.what()));
        }
    } catch (const std::exception& e) { std::cerr << "Error in ink_estimation_benchmark: " << e.what() << std::endl; }
}
