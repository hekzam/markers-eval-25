#include <iostream>
#include <common.h>
#include <cstdlib>
#include <string.h>
#include <random>
#include <tuple>
#include "utils/math_utils.h"
#include "external-tools/modifier.h"

#define MIN_ROTATE -5
#define MAX_ROTATE 5

/**
 * @brief Ajout de bruit poivre et sel
 *
 * @param img Image à modifier                                                                                  //TODO
 * @param max_pepper Pourcentage maximum de poivre
 * @param bright bright = 0 : Neutre; bright > 0 : Éclaircit; bright < 0 : Assombrit
 *
 */
std::optional<std::tuple<int, int>> parse_sp(const std::string& value) {
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

void gestion_arg(cv::Mat& img, int argc, char const* argv[]) {
    // juste sous votre prototype de gestion_arg()
    static const std::vector<std::string> poss_opt = { "-s=", "-g=", "-cb=", "-sp=", "-r=", "-t=", "-nb=" };
    if (argc < 2) {
        std::cerr << "call -> usage() -> exit()" << std::endl;
        exit(1);
    }
    // CAS TTES TRANSFORMATIONS FULL ALEATOIRE
    if (argc == 2) {
        random_exec(img);
        return;
    }
    // CAS TTES TRANSFORMATIONS SEED ALEATOIRES
    for (int i = 2; i < argc; i++) {
        std::string arg(argv[i]);
        size_t pos = arg.find("-seed=");
        if (pos != std::string::npos) {
            std::string str_seed_val = arg.substr(pos + 6);
            if (str_seed_val.empty()) {
                std::cerr << "Erreur : la valeur de -seed= est vide." << std::endl;
            } else {
                try {
                    int value = std::stoi(str_seed_val);
                    std::cout << "Valeur de -seed : " << value << std::endl;
                    int seed = value;
                    random_exec(img, seed);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Erreur : valeur non numérique pour -seed : '" << str_seed_val << "'" << std::endl;
                    // usage()
                } catch (const std::out_of_range& e) {
                    std::cerr << "Erreur : valeur trop grande pour -seed : '" << str_seed_val << "'" << std::endl;
                    // usage()
                }
            }
        }
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
        if (auto res = parse_sp(it->second)) {
            // Structured binding pour décomposer le tuple
            auto [salt, pepper] = *res;
            add_salt_pepper_noise(img, rng, pepper, salt);
        } else {
            std::cerr << "Erreur dans le format de -sp= (attendu: salt, pepper)\n";
            exit(1);
        }
    }
    if (auto it = parsed_opts.find("-g="); it != parsed_opts.end()) {
        if (auto res = parse_sp(it->second)) {
            // Structured binding pour décomposer le tuple
            auto [offset, dispersion] = *res;
            add_gaussian_noise(img, rng, offset, dispersion);
        } else {
            std::cerr << "Erreur dans le format de -g= (attendu: offset, dispersion)\n";
            exit(1);
        }
    }
    if (auto it = parsed_opts.find("-cb="); it != parsed_opts.end()) {
        if (auto res = parse_sp(it->second)) {
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
        if (auto res = parse_sp(it->second)) {
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
    //  identity *= rotate_center(5, img.cols / 2, img.rows / 2);
    // print_mat(identity);
    // identity *= translate(3, 0);
    // print_mat(identity);
    // identity = identity(cv::Rect(0, 0, 3, 2));
    // print_mat(identity);
    // noisy_img = img.clone();
    // add_ink_stain(noisy_img);  // Ajoute le bruit
    // contrast_brightness_modifier(noisy_img,60, 40);                          //SEG_FAULT
    // add_gaussian_noise(noisy_img, 20, 20);
    // add_salt_pepper_noise(noisy_img, 1, 10);
    // cv::warpAffine(noisy_img, calibrated_img, identity, noisy_img.size(), cv::INTER_LINEAR);

    gestion_arg(img, argc, argv);

    cv::imwrite("calibrated_img.png", img);
    return 0;
}