#include "command_line_interface.h"

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
