#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include <common.h>
#include "benchmark_helper.h"
#include "../external-tools/create_copy.h"

std::filesystem::path create_subdir(const std::filesystem::path& base_dir, const std::string& subdir_name) {
    std::filesystem::path subdir_path = base_dir / subdir_name;
    std::filesystem::create_directories(subdir_path);
    return subdir_path;
}

void save_image(cv::Mat img, const std::filesystem::path& output_dir,
                const std::filesystem::path& output_img_path_fname, const std::string& prefix) {
    char* output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/%s%s", output_dir.c_str(), prefix.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, img);
    printf("output image: %s\n", output_img_fname);
    free(output_img_fname);
}

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

bool generate_copies(int nb_copies, int marker_config_id, int encoded_marker_size, int fiducial_marker_size,
                     int grey_level) {
    std::filesystem::path copies_dir = "copies";
    if (std::filesystem::exists(copies_dir)) {
        std::cout << "Cleaning existing copies directory..." << std::endl;
        std::filesystem::remove_all(copies_dir);
    }
    std::filesystem::create_directories(copies_dir);

    std::cout << "Generating " << nb_copies << " copies..." << std::endl;
    bool all_success = true;
    
    // Créer la configuration de style
    CopyStyleParams style_params;
    style_params.encoded_marker_size = encoded_marker_size;
    style_params.fiducial_marker_size = fiducial_marker_size;
    style_params.header_marker_size = 7;  // Valeur par défaut
    style_params.stroke_width = 2;        // Valeur par défaut
    style_params.marker_margin = 5;       // Valeur par défaut
    style_params.grey_level = grey_level;
    
    // Récupérer la configuration des marqueurs
    CopyMarkerConfig marker_config = CopyMarkerConfig::getConfigById(marker_config_id);
    
    for (int i = 1; i <= nb_copies; i++) {
        std::ostringstream copy_name;
        copy_name << "copy" << std::setw(2) << std::setfill('0') << i;

        bool success = create_copy(style_params,     // Paramètres de style
                                   marker_config,    // Configuration des marqueurs
                                   copy_name.str()   // Nom du fichier
        );

        if (!success) {
            std::cerr << "Failed to generate " << copy_name.str() << std::endl;
            all_success = false;
        } else {
            std::cout << "Generated " << copy_name.str() << std::endl;
        }
    }
    return all_success;
}
