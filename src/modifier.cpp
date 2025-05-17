/**
 * @file modifier.cpp
 * @brief Utilitaire en ligne de commande pour appliquer des transformations d'image
 *
 * Cet outil permet d'appliquer diverses transformations et dégradations à une image
 * pour simuler les conditions réelles de numérisation et tester la robustesse des 
 * algorithmes de détection des marqueurs.
 * 
 * ## Modes d'utilisation:
 * 
 * 1. Mode aléatoire simple:
 *    ./modifier input.jpg
 *    Applique une combinaison aléatoire de toutes les transformations
 *    
 * 2. Mode aléatoire avec seed:
 *    ./modifier input.jpg -seed=123
 *    Applique les transformations aléatoires avec une graine spécifique
 *    
 * 3. Mode coefficient de distorsion:
 *    ./modifier input.jpg -coef=0.5
 *    Applique une distorsion proportionnelle au coefficient (entre 0 et 1)
 *    
 * 4. Mode transformations spécifiques:
 *    ./modifier input.jpg -sp=2,3 -cb=50,10 -r=5
 *    Applique uniquement les transformations spécifiées avec les paramètres donnés
 *
 * ## Options disponibles:
 *   -sp=s,p     : Bruit sel & poivre (salt %, pepper %)
 *   -g=d,o      : Bruit gaussien (dispersion, offset)
 *   -cb=c,b     : Contraste/luminosité (contraste, luminosité)
 *   -s=n,min,max: Taches d'encre (nombre, rayon min, rayon max)
 *   -r=angle    : Rotation (angle en degrés)
 *   -t=dx,dy    : Translation (déplacement x, déplacement y)
 *   -seed=n     : Initialisation du générateur aléatoire (nombre entier)
 *   -coef=n     : Coefficient de distorsion (valeur entre 0 et 1)
 */

#include <iostream>
#include <common.h>
#include <cstdlib>
#include <string.h>
#include <random>
#include <tuple>
#include "utils/math_utils.h"
#include "external-tools/modifier.h"


/**
 * @brief Parse une chaîne de caractères pour extraire deux valeurs entières
 *
 * @param value Chaîne de caractères au format "int,int"
 * @return std::optional<std::tuple<int, int>> Tuple contenant les deux valeurs si le parsing a réussi, sinon nullopt
 */
std::optional<std::tuple<int, int>> parse_2(const std::string& value) {
    auto comma = value.find(',');
    if (comma == std::string::npos)
        return std::nullopt;

    try {
        int salt = std::stoi(value.substr(0, comma));
        int pepper = std::stoi(value.substr(comma + 1));
        return std::make_tuple(salt, pepper);
    } catch (...) { return std::nullopt; }
}

/**
 * @brief Parse une chaîne de caractères pour extraire trois valeurs entières
 *
 * @param s Chaîne de caractères au format "int,int,int"
 * @return std::optional<std::tuple<int, int, int>> Tuple contenant les trois valeurs si le parsing a réussi, sinon nullopt
 */
std::optional<std::tuple<int, int, int>> parse_3(const std::string& s) {
    // On cherche deux virgules
    size_t p1 = s.find(',');
    if (p1 == std::string::npos)
        return std::nullopt;
    size_t p2 = s.find(',', p1 + 1);
    if (p2 == std::string::npos)
        return std::nullopt;

    try {
        int nb_spot = std::stoi(s.substr(0, p1));
        int min_radius = std::stoi(s.substr(p1 + 1, p2 - p1 - 1));
        int max_radius = std::stoi(s.substr(p2 + 1));
        return std::make_tuple(nb_spot, min_radius, max_radius);
    } catch (...) { return std::nullopt; }
}

/**
 * @brief Analyse un argument numérique de la ligne de commande
 *
 * @tparam T Type numérique (int ou float)
 * @param argc Nombre d'arguments
 * @param argv Tableau d'arguments
 * @param prefix Préfixe de l'argument à analyser (ex: "-seed=")
 * @param value Variable où stocker la valeur extraite
 * @return true si l'argument a été trouvé et analysé avec succès
 * @return false si l'argument n'a pas été trouvé ou n'a pas pu être analysé
 */
template<typename T>
bool parse_numeric_arg(int argc, char const* argv[], const std::string& prefix, T& value) {
    for (int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.rfind(prefix, 0) == 0) {
            std::string str_val = arg.substr(prefix.size());
            if (str_val.empty()) {
                std::cerr << "Erreur : la valeur de " << prefix << " est vide." << std::endl;
                return false;
            }
            try {
                if constexpr (std::is_same<T, int>::value) {
                    value = std::stoi(str_val);
                } else if constexpr (std::is_same<T, float>::value) {
                    value = std::stof(str_val);
                } else {
                    static_assert(!sizeof(T), "Type non supporté");
                }
                return true;
            } catch (const std::invalid_argument&) {
                std::cerr << "Erreur : valeur non numérique pour " << prefix << " : '" << str_val << "'" << std::endl;
                return false;
            } catch (const std::out_of_range&) {
                std::cerr << "Erreur : valeur hors limites pour " << prefix << " : '" << str_val << "'" << std::endl;
                return false;
            }
        }
    }
    return false;
}

/**
 * @brief Traite les arguments de la ligne de commande et applique les transformations demandées
 *
 * Cette fonction analyse les arguments passés au programme et applique les transformations
 * correspondantes à l'image. Plusieurs modes de fonctionnement sont possibles:
 * - Mode transformations aléatoires complètes (sans arguments supplémentaires)
 * - Mode transformations aléatoires avec seed spécifiée (-seed=)
 * - Mode distorsion avec coefficient (-coef=)
 * - Mode transformations spécifiques (-sp=, -g=, -cb=, -s=, -r=, -t=)
 *
 * @param img L'image à modifier
 * @param argc Nombre d'arguments
 * @param argv Tableau d'arguments
 */
void gestion_arg(cv::Mat& img, int argc, char const* argv[]) {
    // juste sous votre prototype de gestion_arg()
    static const std::vector<std::string> poss_opt = { "-s=", "-g=", "-cb=", "-sp=", "-r=", "-t=", "-nb=" };
    if (argc < 2) {
        std::cerr << "call -> usage() -> exit()" << std::endl;
        exit(1);
    }
    cv::Mat mat;
    // CAS TTES TRANSFORMATIONS FULL ALEATOIRE
    if (argc == 2) {
        random_exec(img, mat);
        return;
    }

    // CAS TTES TRANSFORMATIONS SEED ALEATOIRES

    bool has_seed = false;                      //sert a rien                                     
    int seed_value = 0;
    if (parse_numeric_arg(argc, argv, "-seed=", seed_value)) {
        std::cout << "Valeur de -seed : " << seed_value << std::endl;
        random_exec(img, mat, seed_value);
        has_seed = true;
        return;
    }

    bool has_coef = false;          //sert a rien
    float coef_value = 0.0f;
    if (parse_numeric_arg(argc, argv, "-coef=", coef_value)) {
        std::cout << "Valeur de -coef : " << coef_value << std::endl;
        distorsion_coef_exec(img, mat, coef_value);
        has_coef = true;
        return;
    }
    // CAS UNE OU PLUSIEURS TRANSFORMATIONS AVEC PARAM
    cv::RNG rng(time(0));
    std::map<std::string, std::string> parsed_opts;
    for (int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);
        for (const auto& opt : poss_opt) {
            // Vérifie que l'argument commence par opt
            if (arg.rfind(opt, 0) == 0) {
                // Extrait la valeur après le '='
                std::string value = arg.substr(opt.size());
                // Stocke en mappant l'option à sa valeur
                parsed_opts[opt] = value;
                break; // passe à l'argument suivant
            }
        }
    }
    if (auto it = parsed_opts.find("-sp="); it != parsed_opts.end()) {
        if (auto res = parse_2
        (it->second)) {
            // Structured binding pour décomposer le tuple
            auto [salt, pepper] = *res;
            add_salt_pepper_noise(img, rng, pepper, salt);
        } else {
            std::cerr << "Erreur dans le format de -sp= (attendu: salt, pepper)\n";
            exit(1);
        }
    }
    if (auto it = parsed_opts.find("-g="); it != parsed_opts.end()) {
        if (auto res = parse_2
        (it->second)) {
            // Structured binding pour décomposer le tuple
            auto [offset, dispersion] = *res;
            add_gaussian_noise(img, rng, offset, dispersion);
        } else {
            std::cerr << "Erreur dans le format de -g= (attendu: offset, dispersion)\n";
            exit(1);
        }
    }
    if (auto it = parsed_opts.find("-cb="); it != parsed_opts.end()) {
        if (auto res = parse_2
        (it->second)) {
            // Structured binding pour décomposer le tuple
            auto [contrast, bright] = *res;
            contrast_brightness_modifier(img, contrast, bright);
        } else {
            std::cerr << "Erreur dans le format de -cb= (attendu: contrast, bright)\n";
            exit(1);
        }
    }
    if (auto it = parsed_opts.find("-s="); it != parsed_opts.end()) {
        if (auto res = parse_3(it->second)) {
            // Structured binding pour décomposer le tuple
            auto [nb_spot, min_radius, max_radius] = *res;
            add_ink_stain(img, rng, nb_spot, min_radius, max_radius);
        } else {
            std::cerr << "Erreur dans le format de -s= (attendu: nb spot, min radius, max radius)\n";
            exit(1);
        }
    }
    auto it = parsed_opts.find("-r=");
    if (it != parsed_opts.end()) {
        int angle = std::stof(it->second); // conversion sans surcoût
        rotate_img(img, angle);
    }
    if (auto it = parsed_opts.find("-t="); it != parsed_opts.end()) {
        if (auto res = parse_2
        (it->second)) {
            // Structured binding pour décomposer le tuple
            auto [dx, dy] = *res;
            std::cout << dx << "  " << dy << std::endl;
            translate_img(img, dx, dy);
        } else {
            std::cerr << "Erreur dans le format de -t= (attendu: dx, dy)\n";
            exit(1);
        }
    }

    // si seed not in params
}

/**
 * @brief Point d'entrée principal du programme
 *
 * Charge l'image spécifiée en argument, applique les transformations demandées
 * et enregistre le résultat dans un nouveau fichier.
 *
 * Utilisation: ./modifier <chemin_image> [options]
 * Exemple: ./modifier input.jpg -sp=2,3 -r=5
 *
 * @param argc Nombre d'arguments
 * @param argv Tableau d'arguments
 * @return int Code de retour (0 en cas de succès)
 */
int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_max_pepperth>" << std::endl;
        return 1;
    }
    std::string image_max_pepperth = argv[1];
    cv::Mat img = cv::imread(image_max_pepperth);
    cv::Mat calibrated_img = img.clone();
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    gestion_arg(img, argc, argv);

    cv::imwrite("calibrated_img.png", img);
    return 0;
}