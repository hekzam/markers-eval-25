#include <iostream>
#include <fstream>
#include <map>
#include <variant>
#include <numeric>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"

bool ink_estimation_constraint(const std::map<std::string, Config>& config) {
    // Vérifier si input-dir existe et n'est pas vide
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
 * @brief Calcule la consommation d'encre estimée pour une image en niveaux de gris
 * 
 * @param image Image en niveaux de gris à analyser
 * @param dpi Résolution en DPI
 * @param calibration_factor Facteur de calibration (ml/cm² à 100% couverture)
 * @return double Volume d'encre estimé en ml
 */
double estimate_ink_consumption(const cv::Mat& image, int dpi, double calibration_factor) {
    // 1. Normaliser l'image
    cv::Mat normalized;
    image.convertTo(normalized, CV_32F, 1.0/255.0);
    
    // 2. Calculer la couverture (1.0 - valeur pour chaque pixel)
    cv::Mat coverage = 1.0 - normalized;
    
    // 3. Somme des couvertures relatives
    double ink_units = cv::sum(coverage)[0];
    
    // 4. Surface par pixel en cm²
    double pixel_area_cm2 = pow(2.54 / dpi, 2);
    
    // 5. Surface totale couverte en cm²
    double total_covered_area_cm2 = ink_units * pixel_area_cm2;
    
    // 6. Volume d'encre en ml
    double ink_volume_ml = total_covered_area_cm2 * calibration_factor;
    
    return ink_volume_ml;
}

/**
 * @brief Benchmark d'estimation de consommation d'encre pour une seule image
 */
void ink_estimation_benchmark(const std::map<std::string, Config>& config) {
    try {
        if (!ink_estimation_constraint(config)) {
            return;
        }

        // Vérifier que toutes les clés nécessaires sont présentes dans la configuration
        std::vector<std::string> required_keys = {
            "input-dir", "encoded-marker_size", "unencoded-marker_size", 
            "header-marker_size", "grey-level", "dpi", "marker-config"
        };
        for (const auto& key : required_keys) {
            if (config.find(key) == config.end()) {
                throw std::runtime_error("Missing required configuration key: " + key);
            }
        }

        // Récupération des paramètres avec validation
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
        
        // Validation des paramètres numériques
        if (dpi <= 0) {
            throw std::runtime_error("DPI must be positive");
        }
        if (grey_level < 0 || grey_level > 255) {
            throw std::runtime_error("Grey level must be between 0 and 255");
        }

        // Facteur de calibration par défaut: 0.001 ml/cm² à 100% couverture
        double calibration_factor = 0.001;

        CopyStyleParams style_params;
        style_params = CopyStyleParams(encoded_marker_size, unencoded_marker_size, 7, 2, 5, grey_level, dpi, false);

        std::filesystem::path dir_path{ input_dir };
        if (!std::filesystem::is_directory(dir_path)) {
            throw std::runtime_error("Could not open directory '" + dir_path.string() + "'");
        }
        // Nettoyer le répertoire d'entrée
        std::filesystem::remove_all(dir_path);
        create_copy(style_params, CopyMarkerConfig::getConfigById(marker_config));
        
        // Charger l'image en niveaux de gris depuis le répertoire input_dir
        std::filesystem::path image_path;
        bool found_image = false;
        
        // Parcourir le répertoire pour trouver une image
        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    // Vérifier les extensions d'image courantes
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
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
        
        // Afficher les informations sur l'image
        std::cout << "Image dimensions: " << img.cols << "x" << img.rows << " pixels" << std::endl;
        std::cout << "Resolution: " << dpi << " DPI" << std::endl;
        
        // Estimer la consommation d'encre avec gestion d'erreur
        try {
            cv::Mat normalized;
            img.convertTo(normalized, CV_32F, 1.0/255.0);
            
            if (normalized.empty()) {
                throw std::runtime_error("Failed to normalize image");
            }
            
            cv::Mat coverage = 1.0 - normalized;
            double ink_units = cv::sum(coverage)[0];
            
            // Vérifier les valeurs incohérentes
            if (ink_units < 0) {
                throw std::runtime_error("Calculated negative ink coverage");
            }
            
            double pixel_area_cm2 = pow(2.54 / dpi, 2);
            double total_covered_area_cm2 = ink_units * pixel_area_cm2;
            double ink_volume_ml = total_covered_area_cm2 * calibration_factor;
            
            // Afficher les résultats détaillés
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
    } catch (const std::exception& e) {
        std::cerr << "Error in ink_estimation_benchmark: " << e.what() << std::endl;
    }
}
