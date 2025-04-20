#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

/**
 * @file cli_helper.h
 * @brief Utilitaires pour la gestion de l'interface en ligne de commande.
 *
 * Ce fichier contient des structures et fonctions pour faciliter la gestion
 * des arguments de ligne de commande, l'affichage formaté dans le terminal,
 * et l'interaction avec l'utilisateur via des entrées/sorties console.
 */

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <variant>
#include <common.h>
#include "external-tools/create_copy.h"

/**
 * @brief Structure représentant un paramètre de configuration
 *
 * Cette structure permet de stocker les informations relatives à un paramètre
 * de configuration, y compris son nom, sa description et sa valeur (qui peut être
 * de type entier ou chaîne de caractères).
 */
struct Config {
    std::string name;
    std::string description;
    std::variant<int, std::string> value;
};

/**
 * @brief Affiche l'aide pour les paramètres de configuration disponibles
 *
 * @param default_config Dictionnaire des paramètres de configuration par défaut
 */
void print_help_config(std::unordered_map<std::string, Config> default_config);

/**
 * @brief Extrait les paramètres de configuration depuis les arguments de la ligne de commande
 *
 * @param argc Nombre d'arguments
 * @param argv Tableau des arguments
 * @param default_config Dictionnaire des paramètres de configuration par défaut
 * @return std::optional<std::unordered_map<std::string, Config>> Configuration extraite ou nullopt en cas d'erreur
 */
std::optional<std::unordered_map<std::string, Config>>
get_config(int argc, char* argv[], std::unordered_map<std::string, Config> default_config);

/**
 * @brief Ajoute les paramètres de configuration manquants en demandant à l'utilisateur
 *
 * @param config Dictionnaire de configuration à compléter
 * @param default_config Dictionnaire des paramètres de configuration par défaut
 */
void add_missing_config(std::unordered_map<std::string, Config>& config,
                        const std::unordered_map<std::string, Config>& default_config);

/**
 * @brief Formats de texte disponibles pour l'affichage dans le terminal
 */
enum class TerminalFormat { RESET, BOLD, GREEN, BLUE, CYAN, YELLOW };

/**
 * @brief Convertit un format de terminal en sa représentation string ANSI
 *
 * @param format Format à convertir
 * @return std::string Code ANSI correspondant au format
 */
std::string to_string(TerminalFormat format);

/**
 * @brief Affiche une bannière de bienvenue pour le programme
 *
 * @param title Titre principal à afficher dans la bannière
 * @param subtitle Sous-titre à afficher dans la bannière
 */
void display_banner(const std::string& title = "BENCHMARK TOOL - MARKERS EVALUATION",
                    const std::string& subtitle = "Document Processing & Analysis");

/**
 * @brief Affiche une liste de configurations disponibles
 *
 * @param marker_configs Liste des configurations à afficher
 * @param default_config La configuration par défaut à utiliser
 * @param title Titre à afficher en gras avant la liste
 * @return int La configuration par défaut à utiliser
 */
int display_marker_configs(const std::vector<MarkerConfigInfo>& marker_configs, const int default_config,
                           const std::string& title = "Available configurations:");

/**
 * @brief Affiche une configuration avec formatage coloré
 *
 * @param title Titre de la configuration
 * @param config_pairs Vecteur de paires (nom_paramètre, valeur)
 * @param value_format Format à appliquer aux valeurs (couleur)
 */
void display_configuration_recap(const std::string& title,
                                 const std::vector<std::pair<std::string, std::string>>& config_pairs,
                                 TerminalFormat value_format = TerminalFormat::YELLOW);

/**
 * @brief Vérifie si une chaîne ne contient que des espaces
 *
 * @param str Chaîne à vérifier
 * @return true si la chaîne est vide ou ne contient que des espaces
 * @return false sinon
 */
bool is_whitespace_only(const std::string& str);

/**
 * @brief Demande une entrée à l'utilisateur avec formatage amélioré et validation optionnelle
 *
 * Cette fonction affiche un prompt à l'utilisateur et attend sa saisie. Elle gère
 * l'affichage de la valeur par défaut et peut effectuer une validation sur les valeurs
 * numériques (min/max).
 *
 * @tparam T Type de la valeur à retourner (std::string ou int)
 * @param prompt Message à afficher
 * @param default_value Valeur par défaut
 * @param min_value Valeur minimale autorisée (pour les types numériques)
 * @param max_value Valeur maximale autorisée (pour les types numériques)
 * @return T Valeur entrée par l'utilisateur ou valeur par défaut
 */
template <typename T>
T get_user_input(const std::string& prompt, const T& default_value, const T* min_value = nullptr,
                 const T* max_value = nullptr) {
    std::string input;
    T value = default_value;
    bool valid_input = false;

    do {
        std::cout << to_string(TerminalFormat::BOLD) << prompt << to_string(TerminalFormat::RESET) << " ["
                  << to_string(TerminalFormat::GREEN) << default_value << to_string(TerminalFormat::RESET) << "]: ";
        std::getline(std::cin, input);

        if (input.empty() || is_whitespace_only(input)) {
            value = default_value;
            valid_input = true;
        } else {
            if constexpr (std::is_same_v<T, std::string>) {
                value = input;
                valid_input = true;
            } else if constexpr (std::is_same_v<T, int>) {
                try {
                    value = std::stoi(input);

                    if (min_value && max_value) {
                        if (value >= *min_value && value <= *max_value) {
                            valid_input = true;
                        } else {
                            std::cout << "Value must be between " << *min_value << " and " << *max_value
                                      << ". Please try again." << std::endl;
                        }
                    } else {
                        valid_input = true;
                    }
                } catch (const std::exception& e) { std::cout << "Invalid input. Please enter a number." << std::endl; }
            }
        }
    } while (!valid_input);

    return value;
}

#endif // COMMAND_LINE_INTERFACE_H
