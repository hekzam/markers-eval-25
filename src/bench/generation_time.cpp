#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <tuple>

#include <common.h>

#include "external-tools/create_copy.h"
#include "utils/cli_helper.h"
#include "utils/benchmark_helper.h"
#include "utils/json_helper.h"
#include "generation_time.h"

/**
 * @brief Vérifie et extrait les paramètres de configuration
 * @param config Map de configuration contenant les paramètres
 * @return Tuple contenant les paramètres validés
 * @throws std::invalid_argument Si un paramètre requis est manquant ou invalide
 */
static std::tuple<int, int, int, int, CopyMarkerConfig, int, int> validate_parameters(
    const std::unordered_map<std::string, Config>& config) {
    try {
        int encoded_marker_size = std::get<int>(config.at("encoded-marker_size").value);
        int unencoded_marker_size = std::get<int>(config.at("unencoded-marker_size").value);
        int grey_level = std::get<int>(config.at("grey-level").value);
        int dpi = std::get<int>(config.at("dpi").value);
        auto marker_config = std::get<std::string>(config.at("marker-config").value);
        int nb_copies = std::get<int>(config.at("nb-copies").value);
        int warmup_iterations = std::get<int>(config.at("warmup-iterations").value);
        
        CopyMarkerConfig copy_marker_config;
        if (CopyMarkerConfig::fromString(marker_config, copy_marker_config) != 0) {
            throw std::invalid_argument("Invalid marker configuration: " + marker_config);
        }
        
        return {encoded_marker_size, unencoded_marker_size, grey_level, 
                dpi, copy_marker_config, nb_copies, warmup_iterations};
    } catch (const std::out_of_range& e) {
        throw std::invalid_argument("Missing required parameter in configuration");
    } catch (const std::bad_variant_access& e) {
        throw std::invalid_argument("Invalid parameter type in configuration");
    }
}

void generation_benchmark(const std::unordered_map<std::string, Config>& config) {
    try {
        auto [encoded_marker_size, unencoded_marker_size, grey_level, 
              dpi, copy_marker_config, nb_copies, warmup_iterations] = validate_parameters(config);
        
        CopyStyleParams style_params;
        style_params.encoded_marker_size = encoded_marker_size;
        style_params.unencoded_marker_size = unencoded_marker_size;
        style_params.grey_level = grey_level;
        style_params.dpi = dpi;

        BenchmarkSetup benchmark_setup = prepare_benchmark_directories("./output", false);
        std::ofstream& benchmark_csv = benchmark_setup.benchmark_csv;

        generate_copies(nb_copies, warmup_iterations, style_params, copy_marker_config, benchmark_csv);

        if (benchmark_csv.is_open()) {
            benchmark_csv.close();
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Parameter validation error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in generation_benchmark: " << e.what() << std::endl;
    }
}