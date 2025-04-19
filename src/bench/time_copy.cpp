#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "benchmark.hpp"
#include "utils/math_utils.h"
#include "utils/draw_helper.h"

bool constraint(std::unordered_map<std::string, Config> config) {
    if (std::get<std::string>(config["output-dir"].value).empty() ||
        std::get<std::string>(config["atomic-boxes-file"].value).empty() ||
        std::get<std::string>(config["input-dir"].value).empty()) {
        std::cerr << "Output directory, atomic boxes file and input directory must not be empty" << std::endl;
        return false;
    }

    if (!std::filesystem::exists(std::get<std::string>(config["atomic-boxes-file"].value))) {
        std::cerr << "Atomic boxes file '" << std::get<std::string>(config["atomic-boxes-file"].value)
                  << "' does not exist" << std::endl;
        return false;
    }
    if (std::get<int>(config["nb-copies"].value) <= 0 || std::get<int>(config["nb-copies"].value) > 50) {
        std::cerr << "Number of copies must be between 1 and 50" << std::endl;
        return false;
    }
    return true;
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

void run_benchmark(std::unordered_map<std::string, Config> config) {
    if (!constraint(config)) {
        // print_help_config(default_config_time_copy);
        return;
    }

    int nb_copies = std::get<int>(config["nb-copies"].value);
    int warmup_iterations = std::get<int>(config["warmup-iterations"].value);
    auto atomic_boxes_file = std::get<std::string>(config["atomic-boxes-file"].value);
    auto input_dir = std::get<std::string>(config["input-dir"].value);
    auto copies_str = std::to_string(std::get<int>(config["nb-copies"].value));
    auto encoded_marker_size = std::get<int>(config["encoded-marker_size"].value);
    auto fiducial_marker_size = std::get<int>(config["fiducial-marker_size"].value);
    auto grey_level = std::get<int>(config["grey-level"].value);
    auto dpi = std::get<int>(config["dpi"].value);
    auto marker_config = std::get<int>(config["marker-config"].value);
    ParserType selected_parser = select_parser_for_marker_config(marker_config);

    // Créer une instance de CopyStyleParams avec les paramètres appropriés
    CopyStyleParams style_params;
    style_params = CopyStyleParams(encoded_marker_size, fiducial_marker_size, 7, 2, 5, grey_level, dpi);

    // Generate both warm-up and benchmark copies in a single call
    int total_copies = warmup_iterations + nb_copies;
    if (warmup_iterations > 0) {
        std::cout << "Generating " << total_copies << " copies (" << warmup_iterations << " warm-up + " << nb_copies
                  << " benchmark)..." << std::endl;
    }
    generate_copies(total_copies, marker_config, style_params);

    // Création et nettoyage des répertoires de sortie
    std::filesystem::path output_dir{ std::get<std::string>(config["output-dir"].value) };
    if (std::filesystem::exists(output_dir)) {
        std::cout << "Cleaning existing output directory..." << std::endl;
        std::filesystem::remove_all(output_dir);
    }
    std::filesystem::create_directories(output_dir);
    std::filesystem::path subimg_output_dir = create_subdir(output_dir, "subimg");
    std::filesystem::path csv_output_dir = create_subdir(output_dir, "csv");

    std::filesystem::path benchmark_csv_path = csv_output_dir / "benchmark_results.csv";
    std::ofstream benchmark_csv(benchmark_csv_path);
    if (benchmark_csv.is_open()) {
        benchmark_csv << "File,Time(ms),Success" << std::endl;
    }

    // Lecture et parsing du JSON
    json atomic_boxes_json = parse_json_file(atomic_boxes_file);
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

    std::filesystem::path dir_path{ input_dir };
    if (!std::filesystem::is_directory(dir_path)) {
        throw std::runtime_error("could not open directory '" + dir_path.string() + "'");
    }

    // Collecter tous les fichiers dans un vecteur
    std::vector<std::filesystem::directory_entry> all_entries;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        all_entries.push_back(entry);
    }

    // Vérifier si le nombre d'itérations de warmup est valide
    if (warmup_iterations >= all_entries.size()) {
        std::cout << "Warning: Warmup iterations (" << warmup_iterations
                  << ") is greater than or equal to the number of files (" << all_entries.size() << ")." << std::endl;
        std::cout << "Reducing warmup iterations to " << (all_entries.size() - 1)
                  << " to ensure at least one result is recorded." << std::endl;
        warmup_iterations = (all_entries.size() > 1) ? all_entries.size() - 1 : 0;
    }

    // Afficher les informations sur le warmup
    if (warmup_iterations > 0) {
        std::cout << "Processing " << all_entries.size() << " files - first " << warmup_iterations
                  << " will be used as warm-up iterations" << std::endl;
    }

    int current_iteration = 0;
    const cv::Point2f src_img_size{ 210, 297 };

    for (const auto& entry : all_entries) {
        cv::Mat img = cv::imread(entry.path(), cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);
        auto dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

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
            BenchmarkGuardCSV benchmark_guard(entry.path().filename().string(), &benchmark_csv);
            affine_transform = run_parser(selected_parser, img,
#ifdef DEBUG
                                          debug_img,
#endif
                                          meta, dst_corner_points);
            benchmark_guard.setSuccess(affine_transform.has_value());
        }

        if (!affine_transform.has_value()) {
#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
            fprintf(stderr, "could not find the markers\n");
            current_iteration++;
            continue;
        }

        // Only save results for non-warmup iterations
        if (!is_warmup) {
            auto calibrated_img_col = redress_image(img, affine_transform.value());
            cv::Point2f dimension(calibrated_img_col.cols, calibrated_img_col.rows);

            // Annotation des boîtes utilisateur
            for (auto box : user_boxes_per_page[meta.page - 1]) {
                draw_box_outline(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 255));
            }

            // Annotation des marqueurs de coin
            for (auto box : corner_markers) {
                draw_box_outline(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(255, 0, 0));
                draw_box_center(box, calibrated_img_col, src_img_size, dimension, cv::Scalar(0, 255, 0));
            }

            save_image(calibrated_img_col, output_dir, output_img_path_fname);
#ifdef DEBUG
            save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif
        }

        current_iteration++;
    }

    if (benchmark_csv.is_open()) {
        benchmark_csv.close();
    }

    std::cout << "Benchmark completed with " << warmup_iterations << " warmup iterations and "
              << (all_entries.size() - warmup_iterations) << " measured iterations." << std::endl;
}