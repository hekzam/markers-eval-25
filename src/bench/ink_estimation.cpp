#include <iostream>
#include <fstream>
#include <map>
#include <variant>
#include <numeric>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"

#define INCH 2.54

bool ink_estimation_constraint(const std::map<std::string, Config>& config) {
    if (config.count("input-dir")) {
        auto input_dir = std::get<std::string>(config.at("input-dir").value);
        if (input_dir.empty()) {
            std::cerr << "Input directory must not be empty" << std::endl;
            return false;
        }

        if (!std::filesystem::exists(input_dir)) {
            std::cerr << "Input directory '" << input_dir << "' does not exist" << std::endl;
            return false;
        }
    } else {
        std::cerr << "Input directory must be specified" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Benchmark d'estimation de consommation d'encre pour une seule image
 */
void ink_estimation_benchmark(const std::map<std::string, Config>& config) {
    try {
        if (!ink_estimation_constraint(config)) {
            return;
        }

        std::vector<std::string> required_keys = { "input-dir",          "encoded-marker_size", "unencoded-marker_size",
                                                   "header-marker_size", "grey-level",          "dpi",
                                                   "marker-config" };
        for (const auto& key : required_keys) {
            if (config.find(key) == config.end()) {
                throw std::runtime_error("Missing required configuration key: " + key);
            }
        }

        auto input_dir = std::get<std::string>(config.at("input-dir").value);
        if (input_dir.empty()) {
            throw std::runtime_error("Input directory cannot be empty");
        }

        auto encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
        auto unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
        auto header_marker_size = std::get<int>(config.at("header-marker_size").value);
        auto grey_level = std::get<int>(config.at("grey-level").value);
        auto dpi = std::get<int>(config.at("dpi").value);
        auto marker_config = std::get<int>(config.at("marker-config").value);

        if (dpi <= 0) {
            throw std::runtime_error("DPI must be positive");
        }
        if (grey_level < 0 || grey_level > 255) {
            throw std::runtime_error("Grey level must be between 0 and 255");
        }

        double calibration_factor = 0.001;

        CopyStyleParams style_params(encoded_marker_size, unencoded_marker_size, 7, 2, 5, grey_level, dpi, false);

        std::filesystem::path dir_path{ input_dir };
        if (!std::filesystem::is_directory(dir_path)) {
            throw std::runtime_error("Could not open directory '" + dir_path.string() + "'");
        }
        std::filesystem::remove_all(dir_path);
        create_copy(style_params, CopyMarkerConfig::getConfigById(marker_config));

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
            throw std::runtime_error("No image file found in directory: " + input_dir);
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
