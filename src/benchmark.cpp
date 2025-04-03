#include <unordered_map>
#include <string>
#include <tuple>

#include <common.h>

#include "utils/cli_helper.h"
#include "bench/time_copy.h"

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