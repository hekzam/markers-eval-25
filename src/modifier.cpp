#include <iostream>
#include <common.h>
#include <cstdlib>
#include <string.h>
#include <random>
#include <tuple>
#include "utils/math_utils.h"
#include "external-tools/modifier.h"


/**
 * @brief Ajout de bruit poivre et sel
 *
 * @param img Image à modifier                                                                                  //TODO
 * @param max_pepper Pourcentage maximum de poivre
 * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
 *
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