/**
 * @file benchmark.cpp
 * @brief Module de tests de performance pour l'analyse des copies
 *
 * Ce module permet d'effectuer des tests de performance (benchmarks) sur
 * différentes fonctionnalités du système, notamment sur la génération et
 * l'analyse des copies avec marqueurs. Il fournit une interface en ligne
 * de commande configurable pour différents types de benchmarks.
 */

#include <vector>
#include <unordered_map>
#include <string>

#include <common.h>

#include "utils/cli_helper.h"
#include "bench/parsing_time.h"
#include "bench/generation_time.h"
#include "bench/ink_estimation.h"
#include "external-tools/create_copy.h"

/**
 * @brief Configuration par défaut
 *
 * Ce vecteur ordonné associe chaque paramètre du benchmark à sa configuration complète
 * (nom, description, valeur par défaut).
 */
std::vector<std::pair<std::string, Config>> default_config = {
    { "nb-copies", { "Number of copies", "The number of copies to generate", 1 } },
    { "warmup-iterations", { "Warm-up iterations", "Number of warm-up iterations to run before benchmarking", 0 } },
    { "marker-config",
      { "Marker configuration", "The configuration of the markers to use",
        "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)" } },
    { "encoded-marker_size", { "Encoded marker size", "The size of the encoded markers", 13 } },
    { "unencoded-marker_size", { "Unencoded marker size", "The size of the unencoded markers", 10 } },
    { "header-marker_size", { "Header marker size", "The size of the header marker", 7 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "dpi", { "DPI", "The resolution in dots per inch", 300 } }
};

/**
 * @brief Structure définissant un type de benchmark disponible
 *
 * Cette structure représente un type de benchmark pouvant être exécuté,
 * avec son nom, sa fonction d'exécution et sa configuration par défaut.
 */
struct BenchmarkConfig {
    std::string name;
    void (*run)(const std::unordered_map<std::string, Config>&);
    std::vector<std::pair<std::string, Config>> default_config;
};

/**
 * @brief unordered_map des benchmarks disponibles dans le système
 *
 * Cette unordered_map associe chaque nom de benchmark à sa configuration complète
 * (nom complet, fonction d'exécution, configuration par défaut).
 */
std::unordered_map<std::string, BenchmarkConfig> benchmark_map = {
    { "parsing-time", { "Parsing time benchmark", parsing_benchmark, default_config } },
    { "generation-time", { "Generation time benchmark", generation_benchmark, default_config } },
    { "ink-estimation", { "Ink consumption estimation benchmark", ink_estimation_benchmark, ink_estimation_config } }
};

/**
 * @brief Point d'entrée principal du programme de benchmark
 *
 * Cette fonction affiche la bannière du programme, charge la configuration
 * par défaut pour le benchmark demandé, traite les arguments de ligne de commande
 * pour personnaliser cette configuration, puis exécute le benchmark sélectionné.
 *
 * Si aucun argument n'est fourni, l'utilisateur est invité à choisir un benchmark.
 * Si des arguments invalides sont fournis, l'aide d'utilisation est affichée.
 *
 * @param argc Nombre d'arguments passés au programme
 * @param argv Tableau des arguments passés au programme
 * @return int Code de retour (0 en cas de succès, 1 en cas d'erreur)
 */
int main(int argc, char* argv[]) {
    std::cout << "\n=== Command Line Arguments ===\n";
    std::cout << "Total arguments: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    }
    std::cout << "===========================\n\n";

    std::string benchmark_name = "ink-estimation";
    int benchmark_arg_index = -1;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--benchmark" && i + 1 < argc) {
                benchmark_name = argv[i + 1];
                benchmark_arg_index = i;
                break;
            }
        }
    }

    display_banner("BENCHMARK TOOL - MARKERS EVALUATION",
                   benchmark_map.count(benchmark_name) ? benchmark_map[benchmark_name].name : "Unknown benchmark");

    std::vector<char*> filtered_argv;
    filtered_argv.push_back(argv[0]);

    for (int i = 1; i < argc; i++) {
        if (i == benchmark_arg_index || i == benchmark_arg_index + 1) {
            continue;
        }
        filtered_argv.push_back(argv[i]);
    }

    int filtered_argc = filtered_argv.size();

    auto& selected_default_config =
        benchmark_map.count(benchmark_name) ? benchmark_map[benchmark_name].default_config : default_config;

    auto opt_config = get_config(filtered_argc, filtered_argv.data(), selected_default_config);
    if (!opt_config.has_value()) {
        print_help_config(selected_default_config);
        return 1;
    }
    auto config = opt_config.value();

    if (filtered_argc == 1) {
        add_missing_config(config, selected_default_config);
    } else {
        for (const auto& [key, value] : selected_default_config) {
            if (config.find(key) == config.end()) {
                config[key] = value;
            }
        }
    }

    std::cout << "Running " << benchmark_map[benchmark_name].name << " (" << benchmark_name << ")" << std::endl;
    benchmark_map[benchmark_name].run(config);
    return 0;
}
/// TODO: load page.json