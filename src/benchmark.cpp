#include <unordered_map>
#include <string>

#include <common.h>

#include "utils/cli_helper.h"
#include "bench/time_copy.h"
#include "external-tools/create_copy.h"

std::unordered_map<std::string, Config> default_config_time_copy = {
    { "output-dir", { "Output directory", "The directory where the output images will be saved", "./output" } },
    { "atomic-boxes-file",
      { "Atomic boxes file", "The path to the JSON file containing the atomic boxes", "./original_boxes.json" } },
    { "input-dir", { "Input directory", "The directory containing the input images", "./copies" } },
    { "nb-copies", { "Number of copies", "The number of copies to generate", 1 } },
    { "warmup-iterations", { "Warm-up iterations", "Number of warm-up iterations to run before benchmarking", 0 } },
    { "encoded-marker_size", { "Encoded marker size", "The size of the encoded markers", 15 } },
    { "fiducial-marker_size", { "Fiducial marker size", "The size of the fiducial markers", 10 } },
    { "header-marker_size", { "Header marker size", "The size of the header marker", 7 } },
    { "grey-level", { "Grey level", "The grey level of the markers", 0 } },
    { "dpi", { "DPI", "The resolution in dots per inch", 300 } },
    { "marker-config", { "Marker configuration", "The configuration of the markers", ARUCO_WITH_QR_BR } }
};

struct BenchmarkConfig {
    std::string name;
    void (*run)(std::unordered_map<std::string, Config>);
    std::unordered_map<std::string, Config> default_config;
};

std::unordered_map<std::string, BenchmarkConfig> benchmark_map = {
    { "time-copy", { "Time copy benchmark", run_benchmark, default_config_time_copy } },
};

int main(int argc, char* argv[]) {
    display_banner();

    std::string benchmark_name = "time-copy";

    auto default_config = benchmark_map[benchmark_name].default_config;

    auto opt_config = get_config(argc, argv, default_config);
    if (!opt_config.has_value()) {
        print_help_config(default_config);
        return 1;
    }
    auto config = opt_config.value();
    if (argc == 1) {
        add_missing_config(config, default_config);
    } else {
        for (const auto& [key, value] : default_config) {
            if (config.find(key) == config.end()) {
                config[key] = value;
            }
        }
    }

    benchmark_map[benchmark_name].run(config);
    std::cout << "Benchmark completed." << std::endl;
    return 0;
}
/// TODO: load page.json