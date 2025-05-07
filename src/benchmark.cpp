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
#include <fstream>
#include <sstream>

#include <common.h>

#include "utils/cli_helper.h"
#include "bench/ink_estimation.h"
#include "bench/combined_benchmark.h"
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
    { "encoded-marker-size", { "Encoded marker size", "The size of the encoded markers", 13 } },
    { "unencoded-marker-size", { "Unencoded marker size", "The size of the unencoded markers", 10 } },
    { "header-marker-size", { "Header marker size", "The size of the header marker", 7 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "dpi", { "DPI", "The resolution in dots per inch", 300 } },
    { "parser-type",
      { "Parser type",
        "The type of parser to use (ARUCO, CIRCLE, QRCODE, CUSTOM_MARKER, SHAPE, CENTER_MARKER_PARSER, DEFAULT_PARSER, "
        "EMPTY)",
        std::string("QRCODE") } },
    { "csv-mode",
      { "CSV Mode", "How to handle existing CSV files: 'append' to add data or 'overwrite' to delete and recreate",
        std::string("overwrite") } }
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
    { "ink-estimation", { "Ink consumption estimation benchmark", ink_estimation_benchmark, ink_estimation_config } },
    { "combined", { "Combined generation and parsing benchmark", combined_benchmark, default_config } }
};

/**
 * @brief Exécute une série de benchmarks définis dans un fichier texte
 *
 * @param batch_file Chemin vers le fichier contenant les commandes de benchmark
 * @return int Code de retour (0 en cas de succès, nombre d'erreurs sinon)
 */
int run_batch_benchmarks(const std::string& batch_file) {
    std::ifstream file(batch_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open batch file: " << batch_file << std::endl;
        return 1;
    }

    std::string line;
    int line_number = 0;
    int error_count = 0;

    std::cout << "\n=== Starting Batch Execution from " << batch_file << " ===\n";

    while (std::getline(file, line)) {
        line_number++;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }

        std::cout << "\n=== Executing batch command [" << line_number << "]: " << line << " ===\n";

        // Parse the command line into arguments
        std::vector<std::string> args;
        std::istringstream iss(line);
        std::string arg;

        // Premier mot est le nom du benchmark
        std::string batch_benchmark_name;
        if (!(iss >> batch_benchmark_name)) {
            std::cerr << "Invalid format in batch command line " << line_number << ": empty line" << std::endl;
            error_count++;
            continue;
        }

        // Ajouter --benchmark et le nom du benchmark en tête des arguments
        args.push_back("--benchmark");
        args.push_back(batch_benchmark_name);
        
        // Récupérer le reste des arguments
        while (iss >> arg) {
            args.push_back(arg);
        }

        // Convert to argc/argv style
        int batch_argc = args.size() + 1;
        std::vector<char*> batch_argv;

        // First arg should be program name
        batch_argv.push_back(const_cast<char*>("benchmark"));

        // Add all other args
        for (auto& a : args) {
            batch_argv.push_back(const_cast<char*>(a.c_str()));
        }

        if (benchmark_map.count(batch_benchmark_name) == 0) {
            std::cerr << "Unknown benchmark in batch command: " << batch_benchmark_name << std::endl;
            error_count++;
            continue;
        }

        try {
            // Process arguments like in main()
            auto& selected_default_config = benchmark_map[batch_benchmark_name].default_config;
            
            // Filtrer les arguments pour supprimer --benchmark et le nom du benchmark
            std::vector<char*> filtered_argv;
            filtered_argv.push_back(batch_argv[0]);  // Le nom du programme
            
            // Parcourir tous les arguments sauf --benchmark et le nom du benchmark
            for (int i = 3; i < batch_argc; i++) {
                filtered_argv.push_back(batch_argv[i]);
            }
            
            int filtered_argc = filtered_argv.size();
            
            auto opt_config = get_config(filtered_argc, filtered_argv.data(), selected_default_config);

            if (!opt_config.has_value()) {
                std::cerr << "Invalid configuration in batch command line " << line_number << std::endl;
                error_count++;
                continue;
            }

            auto config = opt_config.value();

            // Fill in missing config with defaults
            for (const auto& [key, value] : selected_default_config) {
                if (config.find(key) == config.end()) {
                    config[key] = value;
                }
            }

            std::cout << "Running " << benchmark_map[batch_benchmark_name].name << " (" << batch_benchmark_name << ")"
                      << std::endl;

            benchmark_map[batch_benchmark_name].run(config);
        } catch (const std::exception& e) {
            std::cerr << "Error in batch command line " << line_number << ": " << e.what() << std::endl;
            error_count++;
        }
    }

    std::cout << "\n=== Batch Execution Complete ===\n";
    std::cout << "Total lines processed: " << line_number << std::endl;
    std::cout << "Errors encountered: " << error_count << std::endl;

    return error_count;
}

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
    std::string benchmark_name = "ink-estimation";
    int benchmark_arg_index = -1;

    std::string batch_file;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--batch-file" && i + 1 < argc) {
            batch_file = argv[i + 1];
            return run_batch_benchmarks(batch_file);
        }
    }

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--benchmark" && i + 1 < argc) {
                benchmark_name = argv[i + 1];
                if (benchmark_map.count(benchmark_name) == 0) {
                    std::cerr << "Unknown benchmark: " << benchmark_name << std::endl;
                    return 1;
                }
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