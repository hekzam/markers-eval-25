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
#include <map>
#include <string>

#include <common.h>

#include "utils/cli_helper.h"
#include "bench/parsing_time.h"
#include "bench/generation_time.h"
#include "external-tools/create_copy.h"

/**
 * @brief Configuration par défaut
 *
 * Ce vecteur ordonné associe chaque paramètre du benchmark à sa configuration complète
 * (nom, description, valeur par défaut) tout en préservant l'ordre d'insertion.
 */
std::vector<std::pair<std::string, Config>> default_config = {
    { "benchmark", { "Benchmark type", "The type of benchmark to run", "generation-time" } },
    { "nb-copies", { "Number of copies", "The number of copies to generate", 1 } },
    { "warmup-iterations", { "Warm-up iterations", "Number of warm-up iterations to run before benchmarking", 0 } },
    { "encoded-marker_size", { "Encoded marker size", "The size of the encoded markers", 15 } },
    { "unencoded-marker_size", { "Fiducial marker size", "The size of the unencoded markers", 10 } },
    { "header-marker_size", { "Header marker size", "The size of the header marker", 7 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "dpi", { "DPI", "The resolution in dots per inch", 300 } },
    { "marker-config", { "Marker configuration", "The configuration of the markers", ARUCO_WITH_QR_BR } },
    { "output-dir", { "Output directory", "The directory where the output images will be saved", "./output" } },
    { "input-dir", { "Input directory", "The directory containing the input images", "./copies" } },
    { "atomic-boxes-file",
      { "Atomic boxes file", "The path to the JSON file containing the atomic boxes", "./original_boxes.json" } }
};

/**
 * @brief Structure définissant un type de benchmark disponible
 *
 * Cette structure représente un type de benchmark pouvant être exécuté,
 * avec son nom, sa fonction d'exécution et sa configuration par défaut.
 */
struct BenchmarkConfig {
    std::string name;
    void (*run)(const std::map<std::string, Config>&);
    std::vector<std::pair<std::string, Config>> default_config;
};

/**
 * @brief Map des benchmarks disponibles dans le système
 *
 * Cette map associe chaque nom de benchmark à sa configuration complète
 * (nom complet, fonction d'exécution, configuration par défaut).
 */
std::map<std::string, BenchmarkConfig> benchmark_map = {
    { "parsing-time", { "Parsing time benchmark", parsing_benchmark, default_config } },
    { "generation-time", { "Generation time benchmark", generation_benchmark, default_config } }
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
    display_banner();

    // Récupérer la configuration avec les arguments de ligne de commande
    auto opt_config = get_config(argc, argv, default_config);
    if (!opt_config.has_value()) {
        print_help_config(default_config);
        return 1;
    }
    auto config = opt_config.value();

    // Compléter la configuration avec les valeurs par défaut
    if (argc == 1) {
        add_missing_config(config, default_config);
    } else {
        for (const auto& [key, value] : default_config) {
            if (config.find(key) == config.end()) {
                config[key] = value;
            }
        }
    }
    
    // Déterminer le benchmark à exécuter
    std::string benchmark_name;
    benchmark_name = std::get<std::string>(config["benchmark"].value);
    
    std::cout << "Running " << benchmark_map[benchmark_name].name << " (" << benchmark_name << ")" << std::endl;
    benchmark_map[benchmark_name].run(config);
    return 0;
}
/// TODO: load page.json