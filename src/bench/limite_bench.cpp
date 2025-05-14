#include <unordered_map>
#include <string>
#include <variant>
#include <common.h>
#include "external-tools/modifier.h"
#include <cli_helper.h>
#include <parser_helper.h>
#include <csv_utils.h>
#include <benchmark_helper.h>
#include <math_utils.h>
#include <json_helper.h>
#include <benchmark.hpp>
#include <draw_helper.h>

#include "limite_bench.h"

typedef std::variant<float, int, std::tuple<int, int, int>, std::tuple<int, int>, std::tuple<float, float>> param_t;

std::string param_to_string(param_t param) {
    if (std::holds_alternative<float>(param)) {
        return std::to_string(std::get<float>(param));
    } else if (std::holds_alternative<int>(param)) {
        return std::to_string(std::get<int>(param));
    } else if (std::holds_alternative<std::tuple<int, int, int>>(param)) {
        auto [a, b, c] = std::get<std::tuple<int, int, int>>(param);
        return "(" + std::to_string(a) + " | " + std::to_string(b) + " | " + std::to_string(c) + ")";
    } else if (std::holds_alternative<std::tuple<int, int>>(param)) {
        auto [a, b] = std::get<std::tuple<int, int>>(param);
        return "(" + std::to_string(a) + " | " + std::to_string(b) + ")";
    } else if (std::holds_alternative<std::tuple<float, float>>(param)) {
        auto [a, b] = std::get<std::tuple<float, float>>(param);
        return "(" + std::to_string(a) + " | " + std::to_string(b) + ")";
    }
    return "";
}

struct option {
    param_t start;
    param_t end;
    param_t (*step)(param_t current, param_t start, param_t end, bool& stop);
    void (*tranformer)(cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                       CopyMarkerConfig marker_config, std::string copy_name, int& margin_size);
};

std::vector<std::pair<std::string, option>> all_config = {
    { "simulate_printer_effects",
      option{ 0.0f, 100.0f,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<float>(current) >= std::get<float>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<float>(current) + 10.0f };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  simulate_printer_effects(img, rng, std::get<float>(param));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "add_salt_pepper_noise",
      option{ 0.0f, 0.3f,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<float>(current) >= std::get<float>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<float>(current) + 0.01f };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  add_salt_pepper_noise(img, rng, std::get<float>(param), std::get<float>(param));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "add_gaussian_noise",
      option{ std::tuple{ 0.0f, 0.0f }, std::tuple{ 100.0f, 100.0f },
              [](param_t current, param_t start, param_t end, bool& stop) {
                  auto [dispersion, offset] = std::get<std::tuple<float, float>>(current);
                  auto [end_dispersion, end_offset] = std::get<std::tuple<float, float>>(end);
                  if (dispersion >= end_dispersion && offset >= end_offset) {
                      stop = true;
                      return current;
                  }
                  dispersion += 10.0f;
                  if (dispersion > end_dispersion) {
                      dispersion = std::get<0>(std::get<std::tuple<float, float>>(start));
                      offset += 10.0f;
                  }
                  return param_t{ std::make_tuple(dispersion, offset) };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  add_gaussian_noise(img, rng, std::get<0>(std::get<std::tuple<float, float>>(param)),
                                     std::get<1>(std::get<std::tuple<float, float>>(param)));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "contrast_brightness_modifier",
      option{ std::tuple{ -100, -100 }, std::tuple{ 100, 100 },
              [](param_t current, param_t start, param_t end, bool& stop) {
                  auto [contrast, bright] = std::get<std::tuple<int, int>>(current);
                  auto [end_contrast, end_bright] = std::get<std::tuple<int, int>>(end);
                  if (contrast >= end_contrast && bright >= end_bright) {
                      stop = true;
                      return current;
                  }
                  contrast += 20;
                  if (contrast > end_contrast) {
                      contrast = std::get<0>(std::get<std::tuple<int, int>>(start));
                      bright += 20;
                  }
                  return param_t{ std::make_tuple(contrast, bright) };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  contrast_brightness_modifier(img, std::get<0>(std::get<std::tuple<int, int>>(param)),
                                               std::get<1>(std::get<std::tuple<int, int>>(param)));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "add_ink_stain",
      option{ std::tuple{ 1, MIN_RMIN, MIN_RMAX }, std::tuple{ MAX_NB_SPOT, MAX_RMIN, MAX_RMAX },
              [](param_t current, param_t start, param_t end, bool& stop) {
                  auto [nombreTaches, rayonMin, rayonMax] = std::get<std::tuple<int, int, int>>(current);
                  auto [end_nombreTaches, end_rayonMin, end_rayonMax] = std::get<std::tuple<int, int, int>>(end);
                  if (nombreTaches >= end_nombreTaches && rayonMin >= end_rayonMin && rayonMax >= end_rayonMax) {
                      stop = true;
                      return current;
                  }
                  nombreTaches += 1;
                  if (nombreTaches > end_nombreTaches) {
                      nombreTaches = std::get<0>(std::get<std::tuple<int, int, int>>(start));
                      rayonMin += 2;
                      if (rayonMin > end_rayonMin) {
                          rayonMin = std::get<1>(std::get<std::tuple<int, int, int>>(start));
                          rayonMax += 5;
                      }
                  }
                  return param_t{ std::make_tuple(nombreTaches, rayonMin, rayonMax) };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  add_ink_stain(img, rng, std::get<0>(std::get<std::tuple<int, int, int>>(param)),
                                std::get<1>(std::get<std::tuple<int, int, int>>(param)),
                                std::get<2>(std::get<std::tuple<int, int, int>>(param)));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "rotate_img",
      option{ -360.0f, 360.0f,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<float>(current) >= std::get<float>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<float>(current) + 15.0f };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  int pixel_offset = std::max(img.cols, img.rows) - std::min(img.cols, img.rows);
                  pixel_offset /= 2;
                  int height = img.rows;
                  int width = img.cols;
                  cv::copyMakeBorder(img, img, pixel_offset, pixel_offset, pixel_offset, pixel_offset,
                                     cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
                  affine_transformation = rotate_center(std::get<float>(param), width / 2.0f, height / 2.0f);
                  affine_transformation = affine_transformation(cv::Rect(0, 0, 3, 2));
                  rotate_img(img, std::get<float>(param));
              } } },
    { "translate_img",
      option{ std::tuple{ -100, -100 }, std::tuple{ 100, 100 },
              [](param_t current, param_t start, param_t end, bool& stop) {
                  auto [dx, dy] = std::get<std::tuple<int, int>>(current);
                  auto [end_dx, end_dy] = std::get<std::tuple<int, int>>(end);
                  if (dx >= end_dx && dy >= end_dy) {
                      stop = true;
                      return current;
                  }
                  dx += 10;
                  if (dx > end_dx) {
                      dx = std::get<0>(std::get<std::tuple<int, int>>(start));
                      dy += 10;
                  }
                  return param_t{ std::make_tuple(dx, dy) };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  auto [dx, dy] = std::get<std::tuple<int, int>>(param);
                  margin_size = std::max(std::abs(dx), std::abs(dy));
                  int height = img.rows;
                  int width = img.cols;
                  cv::copyMakeBorder(img, img, margin_size, margin_size, margin_size, margin_size, cv::BORDER_CONSTANT,
                                     cv::Scalar(255, 255, 255));
                  affine_transformation = (cv::Mat_<float>(2, 3) << 1, 0, dx, 0, 1, dy);
                  translate_img(img, dx, dy);
              } } },
    { "apply_jpeg_compression",
      option{ 0, 100,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 10 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  apply_jpeg_compression(img, std::get<int>(param));
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "encoded_marker_size",
      option{ 3, 25,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 2 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.encoded_marker_size = std::get<int>(param);
                  style.header_marker_size = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "unencoded_marker_size",
      option{ 5, 25,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 2 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.unencoded_marker_size = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "grey_level",
      option{ 0, 255,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 10 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.grey_level = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "dpi",
      option{ 100, 300,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 100 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.dpi = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "stroke_width",
      option{ 1, 10,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 2 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.stroke_width = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
    { "marker_margin",
      option{ 0, 20,
              [](param_t current, param_t start, param_t end, bool& stop) {
                  if (std::get<int>(current) >= std::get<int>(end)) {
                      stop = true;
                      return current;
                  }
                  return param_t{ std::get<int>(current) + 2 };
              },
              [](cv::Mat& img, cv::RNG& rng, param_t param, cv::Mat& affine_transformation, CopyStyleParams style,
                 CopyMarkerConfig marker_config, std::string copy_name, int& margin_size) {
                  style.marker_margin = std::get<int>(param);
                  create_copy(style, marker_config, copy_name, false);
                  img = cv::imread("./copies/" + copy_name + ".png", cv::IMREAD_GRAYSCALE);
                  affine_transformation = cv::Mat::eye(2, 3, CV_32F);
              } } },
};

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
        ParserType selected_parser = ParserType::QRCODE; // Parseur par défaut: QRCODE

        // Si le parseur est spécifié dans la configuration, l'utiliser
        if (config.find("parser-type") != config.end()) {
            std::string parser_type_str = std::get<std::string>(config.at("parser-type").value);
            selected_parser = string_to_parser_type(parser_type_str);

            if (parser_type_str != parser_type_to_string(selected_parser)) {
                std::cout << "Warning: Unknown parser type '" << parser_type_str << "', using default QRCODE parser."
                          << std::endl;
            }
        } else {
            std::cout << "Note: No parser type specified, using default QRCODE parser." << std::endl;
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

void limite_bench(const std::unordered_map<std::string, Config>& config) {
    auto [warmup_iterations, nb_copies, style_params, copy_marker_config, selected_parser, master_seed, csv_mode,
          csv_filename] = validate_parameters(config);

    BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", true, true, csv_mode);

    const cv::Point2f src_img_size{ 210, 297 };

    Csv<std::string, float, int, std::string, CopyMarkerConfig, int, std::string, std::string, double, double, double,
        double, double>
        benchmark_csv(benchmark_setup.csv_output_dir / csv_filename,
                      { "File", "Parsing_Time_ms", "Parsing_Success", "Parser_Type", "Copy_Config", "Seed",
                        "Modification_Type", "Value_Mofication", "Precision_Error_Avg_px", "Precision_Error_TopLeft_px",
                        "Precision_Error_TopRight_px", "Precision_Error_BottomLeft_px",
                        "Precision_Error_BottomRight_px" },
                      csv_mode);

    for (int i = 0; i < warmup_iterations; i++) {
        std::string warmup_copy_name = "warmup" + std::to_string(i + 1);
        create_copy(style_params, copy_marker_config, warmup_copy_name, false);
        std::cout << "  Warmup iteration " << (i + 1) << "/" << warmup_iterations << " completed" << std::endl;
    }

    std::string output_dir = benchmark_setup.output_dir.string();

    cv::RNG rng(master_seed);

    for (const auto& [name, opt] : all_config) {
        param_t current = opt.start;
        param_t end = opt.end;
        bool stop = false;
        while (!stop) {
            for (int i = 0; i < nb_copies; i++) {
                // std::string copy_name = name + "_" + param_to_string(current);
                std::string copy_name = name + "_" + std::to_string(i + 1) + "_" + param_to_string(current);
                cv::Mat img;
                cv::Mat mat;
                int margin_size = 0;
                opt.tranformer(img, rng, current, mat, style_params, copy_marker_config, copy_name, margin_size);
                std::string copy_full_name = copy_name + ".png";
                cv::imwrite("./copies/" + copy_full_name, img);
                std::cout << "  Copy " << copy_name << " created" << std::endl;
                img = cv::imread("./copies/" + copy_full_name, cv::IMREAD_GRAYSCALE);
                if (img.empty()) {
                    std::cerr << "Error: Could not read image " << copy_full_name << std::endl;
                    continue;
                }

                // std::string local_copy_name = copy_name + "_" + std::to_string(i + 1) + ".png";
                std::string local_copy_name = copy_full_name;

                json atomic_boxes_json = parse_json_file("./original_boxes.json");
                auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
                std::vector<std::optional<std::shared_ptr<AtomicBox>>> corner_markers;
                std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
                differentiate_atomic_boxes(atomic_boxes, corner_markers, user_boxes_per_page);

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

                std::filesystem::path output_img_path_fname = std::filesystem::path(local_copy_name);

#ifdef DEBUG
                save_debug_img(debug_img, output_dir, output_img_path_fname);
#endif

                std::vector<double> precision_errors = { -1.0, -1.0, -1.0, -1.0,
                                                         -1.0 }; // -1.0 pour indiquer une erreur
                if (parsing_success) {
                    auto calibrated_img_col = redress_image(img, affine_transform.value());

                    precision_errors =
                        calculate_precision_error(dst_img_size, mat, affine_transform.value(), margin_size);
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
                benchmark_csv.add_row({ copy_name, parsing_milliseconds, parsing_success ? 1 : 0,
                                        parser_type_to_string(selected_parser), copy_marker_config, master_seed, name,
                                        param_to_string(current), precision_errors.back(), precision_errors[0],
                                        precision_errors[1], precision_errors[2], precision_errors[3] });
            }
            current = opt.step(current, opt.start, opt.end, stop);
        }
    }
}