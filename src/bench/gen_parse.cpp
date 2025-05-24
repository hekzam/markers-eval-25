#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <tuple>
#include <chrono>
#include <random>
#include <cmath>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "utils/parser_helper.h"
#include "utils/math_utils.h"
#include "utils/draw_helper.h"
#include "gen_parse.h"
#include <modifier.h>

struct CopyInfo {
    std::string filename;
    double generation_time;
};

/**
 * @brief Vérifie et extrait les paramètres de configuration
 * @param config Map de configuration contenant les paramètres
 * @return Tuple contenant les paramètres validés
 * @throws std::invalid_argument Si un paramètre requis est manquant ou invalide
 */
static std::tuple<int, int, CopyStyleParams, CopyMarkerConfig, ParserType, int, CsvMode, std::string>
validate_parameters(const std::unordered_map<std::string, Config>& config) {
    try {
        int warmup_iterations = std::get<int>(config.at("warmup-iterations").value);
        int nb_copies = std::get<int>(config.at("nb-copies").value);
        int encoded_marker_size = std::get<int>(config.at("encoded-marker-size").value);
        int unencoded_marker_size = std::get<int>(config.at("unencoded-marker-size").value);
        int header_marker_size = std::get<int>(config.at("header-marker-size").value);
        int grey_level = std::get<int>(config.at("grey-level").value);
        int dpi = std::get<int>(config.at("dpi").value);
        int master_seed = std::get<int>(config.at("seed").value);
        auto marker_config = std::get<std::string>(config.at("marker-config").value);

        CopyMarkerConfig copy_marker_config;
        if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
            throw std::invalid_argument("Invalid marker configuration: " + marker_config);
        }

        // Permettre à l'utilisateur de spécifier explicitement le parseur
        ParserType selected_parser = ParserType::ZXING; // Parseur par défaut: ZXING

        // Si le parseur est spécifié dans la configuration, l'utiliser
        if (config.find("parser-type") != config.end()) {
            std::string parser_type_str = std::get<std::string>(config.at("parser-type").value);
            selected_parser = string_to_parser_type(parser_type_str);

            if (parser_type_str != parser_type_to_string(selected_parser)) {
                std::cout << "Warning: Unknown parser type '" << parser_type_str << "', using default ZXING parser."
                          << std::endl;
            }
        } else {
            std::cout << "Note: No parser type specified, using default ZXING parser." << std::endl;
        }

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

        std::string csv_filename = "benchmark_results.csv";
        if (config.find("csv-filename") != config.end()) {
            csv_filename = std::get<std::string>(config.at("csv-filename").value);
            std::cout << "CSV Filename: " << csv_filename << std::endl;
        }

        CopyStyleParams style_params;
        style_params.encoded_marker_size = encoded_marker_size;
        style_params.unencoded_marker_size = unencoded_marker_size;
        style_params.header_marker_size = header_marker_size;
        style_params.grey_level = grey_level;
        style_params.dpi = dpi;

        return { warmup_iterations, nb_copies,   style_params, copy_marker_config,
                 selected_parser,   master_seed, csv_mode,     csv_filename };
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

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < warmup_iterations; i++) {
        std::string warmup_copy_name = "warmup" + std::to_string(i + 1);

        int warmup_seed = gen();

        CopyStyleParams local_style_params = style_params;
        local_style_params.seed = warmup_seed;

        create_copy(local_style_params, copy_marker_config, warmup_copy_name, false);
        std::cout << "  Warmup iteration " << (i + 1) << "/" << warmup_iterations
                  << " completed with seed: " << warmup_seed << std::endl;
    }
}

std::vector<CopyInfo> bench_copy(int nb_copies, const CopyStyleParams& style_params,
                                 const CopyMarkerConfig& copy_marker_config, std::mt19937& master_gen) {
    std::vector<CopyInfo> generated_copies;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1000, 9999);

    for (int i = 1; i <= nb_copies; i++) {
        std::cout << "Generating copy " << i << "/" << nb_copies << "..." << std::endl;

        int random_suffix = dist(gen);
        std::string copy_name = "copy-" + std::to_string(i) + "-" + std::to_string(random_suffix);
        std::string filename = copy_name + ".png";

        int content_seed = master_gen();

        CopyStyleParams local_style_params = style_params;
        local_style_params.seed = content_seed;

        bool copy_success = false;
        auto create_copy_lambda = [&]() { copy_success = create_copy(local_style_params, copy_marker_config, copy_name, false); };

        double milliseconds = Benchmark::measure("  Generation time", create_copy_lambda);
        
        if (!copy_success) {
            std::cerr << "Error: Failed to create copy " << i << "/" << nb_copies << " with filename: " << filename << std::endl;
            throw std::runtime_error("Copy creation failed, stopping benchmark");
        }

        generated_copies.push_back({ filename, milliseconds });
    }
    return generated_copies;
}

void warmup_parsing(int warmup_iterations, const cv::Point2f& src_img_size, ParserType selected_parser,
                    const CopyMarkerConfig& copy_marker_config) {
    if (warmup_iterations > 0) {
        std::cout << "\nParsing warmup iterations..." << std::endl;
    } else {
        std::cout << "No warmup iterations requested." << std::endl;
        return;
    }

    for (int i = 0; i < warmup_iterations; i++) {
        std::string warmup_copy_name = "warmup" + std::to_string(i + 1);
        std::string warmup_filename = warmup_copy_name + ".png";
        std::cout << "Parsing warmup copy: " << warmup_filename << "..." << std::endl;

        // Récupérer le fichier de métadonnées pour cette copie d'échauffement
        std::string json_path = get_metadata_path(warmup_copy_name);
        std::cout << "  Loading metadata from: " << json_path << std::endl;

        // Vérifier si le fichier JSON existe
        if (!std::filesystem::exists(json_path)) {
            std::cerr << "  Error: Metadata file not found for warmup: " << json_path << std::endl;
            continue;
        }

        // Lire les métadonnées spécifiques à cette copie d'échauffement
        json atomic_boxes_json = parse_json_file(json_path);
        if (atomic_boxes_json.empty()) {
            std::cerr << "  Error: Failed to parse metadata for warmup: " << json_path << std::endl;
            continue;
        }

        std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
        std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;

        auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
        differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

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

        std::cout << "  Warmup parsing " << (i + 1) << "/" << warmup_iterations << " completed with "
                  << (transform.has_value() ? "success" : "failure") << std::endl;
    }
}

void bench_parsing(std::vector<CopyInfo>& generated_copies, const cv::Point2f& src_img_size, ParserType selected_parser,
                   const CopyMarkerConfig& copy_marker_config,
                   Csv<std::string, double, double, int, std::string, CopyMarkerConfig, int, double, double, double,
                       double, double>& benchmark_csv,
                   std::string output_dir, std::mt19937& master_gen) {
    std::random_device rd;
    std::mt19937 gen(rd());

    auto add_error_to_csv = [&](const CopyInfo& copy_info, int seed = -1) {
        benchmark_csv.add_row({ copy_info.filename, copy_info.generation_time, 0, 0,
                                parser_type_to_string(selected_parser), copy_marker_config, seed, 
                                std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan("") });
    };

    for (const auto& copy_info : generated_copies) {
        std::cout << "Parsing copy: " << copy_info.filename << "..." << std::endl;

        std::string filename_without_ext = copy_info.filename.substr(0, copy_info.filename.find_last_of('.'));
        std::string json_path = get_metadata_path(filename_without_ext);
        std::cout << "  Loading metadata from: " << json_path << std::endl;

        if (!std::filesystem::exists(json_path)) {
            std::cerr << "  Error: Metadata file not found: " << json_path << std::endl;
            add_error_to_csv(copy_info);
            continue;
        }

        json atomic_boxes_json = parse_json_file(json_path);
        if (atomic_boxes_json.empty()) {
            std::cerr << "  Error: Failed to parse metadata from: " << json_path << std::endl;
            add_error_to_csv(copy_info);
            continue;
        }

        std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
        std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;

        auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
        differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

        cv::Mat img = cv::imread("./copies/" + copy_info.filename, cv::IMREAD_GRAYSCALE);
        if (!img.data) {
            std::cerr << "Error: Could not read generated image: " << copy_info.filename << std::endl;
            add_error_to_csv(copy_info);
            continue;
        }

        const cv::Point2f original_img_size(img.cols, img.rows);

        cv::Mat mat;
        int unique_seed = master_gen();
        random_exec(img, mat, unique_seed);

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
        parsing_success = parsing_success && meta.id == 0;
        parsing_success = parsing_success && meta.page > 0;
        std::cout << "  Success: " << (parsing_success ? "Yes" : "No") << std::endl;

        std::filesystem::path output_img_path_fname = std::filesystem::path(copy_info.filename);

#ifdef DEBUG
        save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif

        std::vector<double> precision_errors = { 
            std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan("")
        };
        if (parsing_success) {
            auto calibrated_img_col = redress_image(img, affine_transform.value());

            precision_errors =
                calculate_precision_error(dst_img_size, mat, affine_transform.value(), MARGIN_COPY_MODIFIED);
            std::cout << "  Precision error: " << std::fixed << std::setprecision(3) << precision_errors.back()
                      << " pixels" << std::endl;

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
                                unique_seed, precision_errors.back(), precision_errors[0], precision_errors[1],
                                precision_errors[2], precision_errors[3] });
    }
}

void gen_parse(const std::unordered_map<std::string, Config>& config) {
    auto [warmup_iterations, nb_copies, style_params, copy_marker_config, selected_parser, master_seed, csv_mode,
          csv_filename] = validate_parameters(config);

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", true, true, csv_mode);

    Csv<std::string, double, double, int, std::string, CopyMarkerConfig, int, double, double, double, double, double>
        benchmark_csv(benchmark_setup.csv_output_dir / csv_filename,
                      { "File", "Generation_Time_ms", "Parsing_Time_ms", "Parsing_Success", "Parser_Type",
                        "Copy_Config", "Seed", "Precision_Error_Avg_px", "Precision_Error_TopLeft_px",
                        "Precision_Error_TopRight_px", "Precision_Error_BottomLeft_px",
                        "Precision_Error_BottomRight_px" },
                      csv_mode);

    std::cout << "ÉTAPE 1: Génération des copies..." << std::endl;

    warmup_copy(warmup_iterations, style_params, copy_marker_config);

    std::mt19937 master_gen;
    if (master_seed != 0) {
        std::cout << "Using master seed: " << master_seed << std::endl;
        master_gen.seed(master_seed);
    } else {
        std::random_device rd;
        master_gen.seed(rd());
        std::cout << "No master seed provided, using random initialization" << std::endl;
    }

    auto generated_copies = bench_copy(nb_copies, style_params, copy_marker_config, master_gen);

    if (generated_copies.empty()) {
        throw std::runtime_error("No copies were generated");
    }

    const cv::Point2f src_img_size{ 210, 297 };

    warmup_parsing(warmup_iterations, src_img_size, selected_parser, copy_marker_config);

    std::cout << "\nÉTAPE 2: Parsing des copies générées..." << std::endl;

    bench_parsing(generated_copies, src_img_size, selected_parser, copy_marker_config, benchmark_csv,
                  benchmark_setup.output_dir, master_gen);

    std::cout << "gen-parse benchmark completed with " << warmup_iterations << " warmup iterations and " << nb_copies
              << " copies." << std::endl;
}