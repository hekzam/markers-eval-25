#include "cli_helper.h"

void print_help_config(std::unordered_map<std::string, Config> default_config) {
    for (const auto& [key, config] : default_config) {
        if (std::holds_alternative<int>(config.value)) {
            std::cout << "--" << key << " <int>: " << config.description << " (default: " << std::get<int>(config.value)
                      << ")" << std::endl;
        } else {
            std::cout << "--" << key << " <string>: " << config.description
                      << " (default: " << std::get<std::string>(config.value) << ")" << std::endl;
        }
    }
}

std::optional<std::unordered_map<std::string, Config>>
get_config(int argc, char* argv[], std::unordered_map<std::string, Config> default_config) {
    std::unordered_map<std::string, Config> config = {};
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) != "--") {
            std::cerr << "Invalid argument: " << arg << std::endl;
            return {};
        }
        arg = arg.substr(2);
        if (default_config.find(arg) == default_config.end()) {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return {};
        }
        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument: " << arg << std::endl;
            return {};
        }
        std::string value = argv[i + 1];
        if (std::holds_alternative<int>(default_config[arg].value)) {
            try {
                config[arg].value = std::stoi(value);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid value for argument " << arg << ": " << value << std::endl;
            }
        } else {
            config[arg].value = value;
        }
        i++;
    }
    return config;
}

void add_missing_config(std::unordered_map<std::string, Config>& config,
                        const std::unordered_map<std::string, Config>& default_config) {
    for (const auto& [key, default_config] : default_config) {
        if (config.find(key) != config.end()) {
            continue;
        }
        if (std::holds_alternative<int>(default_config.value)) {
            config[key].value = get_user_input(default_config.name, std::get<int>(default_config.value));
        } else {
            config[key].value = get_user_input(default_config.name, std::get<std::string>(default_config.value));
        }
    }
}

std::string to_string(TerminalFormat format) {
    switch (format) {
        case TerminalFormat::RESET:
            return "\033[0m";
        case TerminalFormat::BOLD:
            return "\033[1m";
        case TerminalFormat::GREEN:
            return "\033[32m";
        case TerminalFormat::BLUE:
            return "\033[34m";
        case TerminalFormat::CYAN:
            return "\033[36m";
        case TerminalFormat::YELLOW:
            return "\033[33m";
        default:
            return "";
    }
}

void display_banner(const std::string& title, const std::string& subtitle) {
    std::cout << std::string(50, '-') << std::endl;
    std::cout << to_string(TerminalFormat::BOLD) << to_string(TerminalFormat::BLUE) << "  " << title
              << to_string(TerminalFormat::RESET) << std::endl;
    std::cout << to_string(TerminalFormat::CYAN) << "  " << subtitle << to_string(TerminalFormat::RESET) << std::endl;
    std::cout << std::string(50, '-') << std::endl << std::endl;
}

int display_marker_configs(const std::vector<MarkerConfigInfo>& marker_configs, const int default_config,
                           const std::string& title) {
    std::cout << std::endl << to_string(TerminalFormat::BOLD) << title << to_string(TerminalFormat::RESET) << std::endl;

    for (const auto& config : marker_configs) {
        std::cout << std::setw(3) << config.id << ". " << config.description << std::endl;
    }

    return default_config;
}

void display_configuration_recap(const std::string& title,
                                 const std::vector<std::pair<std::string, std::string>>& config_pairs,
                                 TerminalFormat value_format) {
    std::cout << std::endl << to_string(TerminalFormat::BOLD) << title << to_string(TerminalFormat::RESET) << std::endl;

    for (const auto& [key, value] : config_pairs) {
        std::cout << "- " << key << ": " << to_string(value_format) << value << to_string(TerminalFormat::RESET)
                  << std::endl;
    }

    std::cout << std::endl;
}

bool is_whitespace_only(const std::string& str) {
    return str.empty() || str.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
}
