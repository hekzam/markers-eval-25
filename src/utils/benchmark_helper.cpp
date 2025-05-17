#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <optional>
#include <functional>

#include <common.h>
#include "benchmark_helper.h"
#include "../external-tools/create_copy.h"
#include "../utils/cli_helper.h"
#include "benchmark.hpp"
#include "csv_utils.h"
#include "math_utils.h"

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

void clean_directory(const std::string& dir) {
    if (std::filesystem::exists(dir)) {
        std::cout << "Cleaning existing directory: " << dir << std::endl;
        std::filesystem::remove_all(dir);
    }
    std::filesystem::create_directories(dir);
}

void clean_directory_preserve_csv(const std::string& dir, const std::filesystem::path& csv_dir, CsvMode csv_mode) {
    if (std::filesystem::exists(dir)) {
        std::cout << "Cleaning directory: " << dir << std::endl;

        if (csv_mode == CsvMode::APPEND && std::filesystem::exists(csv_dir)) {
            std::filesystem::path dir_path(dir);
            std::filesystem::path temp_csv_dir = dir_path / "temp_csv_backup";
            std::filesystem::create_directories(temp_csv_dir);

            for (const auto& entry : std::filesystem::directory_iterator(csv_dir)) {
                if (entry.path().extension() == ".csv") {
                    std::filesystem::copy(entry.path(), temp_csv_dir / entry.path().filename());
                }
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (entry.path() != temp_csv_dir) {
                    std::filesystem::remove_all(entry.path());
                }
            }

            std::filesystem::create_directories(csv_dir);

            for (const auto& entry : std::filesystem::directory_iterator(temp_csv_dir)) {
                std::filesystem::copy(entry.path(), csv_dir / entry.path().filename(),
                                      std::filesystem::copy_options::overwrite_existing);
            }

            std::filesystem::remove_all(temp_csv_dir);
        } else {
            std::filesystem::remove_all(dir);
            std::filesystem::create_directories(dir);
        }
    } else {
        std::filesystem::create_directories(dir);
    }
}

BenchmarkSetup prepare_benchmark_directories(const std::string& output_dir, bool include_success_column,
                                             bool create_subimg_dir, CsvMode csv_mode) {
    BenchmarkSetup setup;

    setup.output_dir = std::filesystem::path{ output_dir };
    setup.csv_output_dir = setup.output_dir / "csv";

    clean_directory_preserve_csv(setup.output_dir, setup.csv_output_dir, csv_mode);

    if (create_subimg_dir) {
        setup.subimg_output_dir = create_subdir(setup.output_dir, "subimg");
    }

    std::filesystem::create_directories(setup.csv_output_dir);

    std::filesystem::path copies_dir = "./copies";
    clean_directory(copies_dir);

    return setup;
}

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
                      const cv::Point2f& dst_size, const cv::Scalar& color, int thickness) {
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
                     const cv::Point2f& dst_size, const cv::Scalar& color, int radius, int thickness) {
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
std::vector<cv::Point2f> calculate_theoretical_corners(const cv::Point2f& dst_img_size) {
    // Les coordonnées théoriques des coins d'une page A4 en mm (0,0 est en haut à gauche)
    std::vector<cv::Point2f> theoretical_corners = {
        { 0, 0 },                          // Haut-gauche
        { dst_img_size.x, 0 },             // Haut-droit
        { 0, dst_img_size.y },             // Bas-gauche
        { dst_img_size.x, dst_img_size.y } // Bas-droit
    };
    return theoretical_corners;
}

/**
 * @brief Calcule l'erreur de précision entre les coins originaux et les coins calibrés
 *
 * @param src_img_size Dimensions théoriques de l'image source (210x297 mm)
 * @param dst_img_size Dimensions de l'image calibrée en pixels
 * @param transform_matrix Matrice de transformation appliquée à l'image
 * @param rectification_transform Matrice de transformation affine du parser
 * @param margin Marge d'erreur en pixels
 * @return std::vector<double> Erreurs de précision pour chaque coin (haut-gauche, haut-droit, bas-gauche, bas-droit) et
 * la moyenne en dernière position
 */
std::vector<double> calculate_precision_error(const cv::Point2f& dst_img_size, const cv::Mat& transform_matrix,
                                              const cv::Mat& rectification_transform, float margin) {
    std::vector<cv::Point2f> original_corners = calculate_theoretical_corners(dst_img_size);

    std::vector<cv::Point2f> transformed_corner = original_corners;
    for (auto& corner : transformed_corner) {
        // Ajuster les coordonnées pour compenser la marge
        if (corner.x < dst_img_size.x / 2)
            corner.x += margin;
        else
            corner.x -= margin;

        if (corner.y < dst_img_size.y / 2)
            corner.y += margin;
        else
            corner.y -= margin;
    }
    cv::transform(transformed_corner, transformed_corner, transform_matrix);

    cv::transform(transformed_corner, transformed_corner, rectification_transform);

    std::vector<double> distances;
    double total_distance = 0.0;

    for (size_t i = 0; i < 4; i++) {
        double distance = euclidean_distance(original_corners[i], transformed_corner[i]);

        std::cout << "  Corner " << i << " - Original: (" << original_corners[i].x << ", " << original_corners[i].y
                  << "), after Transformation: (" << transformed_corner[i].x << ", " << transformed_corner[i].y
                  << "), Difference x: " << transformed_corner[i].x - original_corners[i].x
                  << ", Difference y: " << transformed_corner[i].y - original_corners[i].y << ", Distance: " << distance
                  << " pixels" << std::endl;

        total_distance += distance;
        distances.push_back(distance);
    }

    if (!distances.empty()) {
        double avg_error = total_distance / distances.size();
        std::cout << "  Average precision error: " << avg_error << " pixels" << std::endl;
        distances.push_back(avg_error);
        return distances;
    }

    return { -1.0, -1.0, -1.0, -1.0, -1.0 };
}

std::string get_metadata_path(const std::string& filename) {
    return "./copies/metadata/" + filename + ".json";
}