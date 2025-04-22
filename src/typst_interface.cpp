/**
 * @file typst_interface.cpp
 * @brief Interface entre Typst et le générateur de copies
 *
 * Ce module permet d'interfacer le système de génération de documents Typst
 * avec le générateur de copies marquées. Il analyse les arguments passés en ligne
 * de commande et configure les marqueurs selon les spécifications.
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include "external-tools/create_copy.h"

/**
 * @brief Vérifie si la configuration de marqueur spécifiée est valide
 *
 * @param config Numéro de configuration à vérifier
 * @return bool Retourne vrai si la configuration est valide (entre QR_ALL_CORNERS et SQUARE_OUTLINES_WITH_QR_BR)
 */
bool isValidMarkerConfig(int config) {
    return config >= QR_ALL_CORNERS && config <= SQUARE_OUTLINES_WITH_QR_BR;
}

/**
 * @brief Convertit une chaîne de caractères en type MarkerType
 *
 * Cette fonction prend une représentation textuelle d'un type de marqueur
 * et la convertit en son équivalent dans l'énumération MarkerType.
 *
 * @param typeStr Chaîne représentant le type de marqueur
 * @return MarkerType Type de marqueur correspondant, ou NONE si non reconnu
 */
MarkerType markerTypeFromString(const std::string& typeStr) {
    static const std::unordered_map<std::string, MarkerType> markerTypeMap = { { "qrcode", MarkerType::QR_CODE },
                                                                               { "datamatrix", MarkerType::DATAMATRIX },
                                                                               { "aztec", MarkerType::AZTEC },
                                                                               { "pdf417", MarkerType::PDF417 },
                                                                               { "rmqr", MarkerType::RMQR },
                                                                               { "code128", MarkerType::BARCODE },
                                                                               { "circle", MarkerType::CIRCLE },
                                                                               { "square", MarkerType::SQUARE },
                                                                               { "triangle", MarkerType::TRIANGLE },
                                                                               { "aruco", MarkerType::ARUCO },
                                                                               { "custom", MarkerType::CUSTOM },
                                                                               { "qr-eye", MarkerType::QR_EYE },
                                                                               { "cross", MarkerType::CROSS },
                                                                               { "micro-qr",
                                                                                 MarkerType::MICRO_QR_CODE },
                                                                               { "none", MarkerType::NONE } };

    auto it = markerTypeMap.find(typeStr);
    if (it != markerTypeMap.end()) {
        return it->second;
    }

    // Par défaut, si type inconnu, on retourne NONE
    return MarkerType::NONE;
}

/**
 * @brief Analyse une spécification de marqueur et crée l'objet Marker correspondant
 *
 * Cette fonction prend une chaîne de caractères qui décrit un marqueur
 * (par exemple "qrcode:encoded" ou "circle:outlined") et retourne
 * un objet Marker configuré selon cette spécification.
 *
 * @param spec Chaîne de spécification du marqueur au format "type[:encoded][:outlined]"
 * @return Marker Objet Marker configuré selon la spécification, ou marqueur vide si spec est vide ou "none"
 */
Marker parseMarker(const std::string& spec) {
    if (spec.empty() || spec == "none") {
        return Marker();
    }

    std::string type = spec;
    bool encoded = false;
    bool outlined = false;

    size_t encodedPos = spec.find(":encoded");
    if (encodedPos != std::string::npos) {
        encoded = true;
        type = spec.substr(0, encodedPos);
    }

    size_t outlinedPos = spec.find(":outlined");
    if (outlinedPos != std::string::npos) {
        outlined = true;
        if (encodedPos != std::string::npos && outlinedPos < encodedPos) {
            type = spec.substr(0, outlinedPos);
        } else if (encodedPos == std::string::npos) {
            type = spec.substr(0, outlinedPos);
        }
    }

    return Marker(markerTypeFromString(type), encoded, outlined);
}

/**
 * @brief Point d'entrée principal du programme
 *
 * Cette fonction analyse les arguments de la ligne de commande pour configurer
 * et générer une copie avec les marqueurs spécifiés. Les arguments supportés sont :
 * --encoded-size, --unencoded-size, --stroke-width, --margin, --config,
 * --tl, --tr, --bl, --br, --header, --grey-level, --header-size, --filename, --dpi
 *
 * @param argc Nombre d'arguments passés au programme
 * @param argv Tableau des arguments passés au programme
 * @return int Code de retour (0 en cas de succès, 1 en cas d'erreur)
 */
int main(int argc, char* argv[]) {
    CopyStyleParams style_params;
    int marker_config = -1;
    std::string filename = "copy";

    Marker top_left, top_right, bottom_left, bottom_right, header;
    bool custom_markers = false;

    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];

        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument: " << arg << std::endl;
            return 1;
        }

        if (arg == "--encoded-size") {
            style_params.encoded_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--unencoded-size") {
            style_params.unencoded_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--stroke-width") {
            style_params.stroke_width = std::atoi(argv[i + 1]);
        } else if (arg == "--margin") {
            style_params.marker_margin = std::atoi(argv[i + 1]);
        } else if (arg == "--config") {
            marker_config = std::atoi(argv[i + 1]);
            if (!isValidMarkerConfig(marker_config)) {
                std::cerr << "Invalid marker configuration: " << argv[i + 1] << " (valid range: 1-10)" << std::endl;
                return 1;
            }
        } else if (arg == "--generating-content") {
            style_params.generating_content = (std::string(argv[i + 1]) == "1" || std::string(argv[i + 1]) == "true");
        } else if (arg == "--tl") {
            top_left = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--tr") {
            top_right = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--bl") {
            bottom_left = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--br") {
            bottom_right = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--header") {
            header = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--grey-level") {
            style_params.grey_level = std::atoi(argv[i + 1]);
        } else if (arg == "--header-size") {
            style_params.header_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--filename") {
            filename = argv[i + 1];
        } else if (arg == "--dpi") {
            style_params.dpi = std::atoi(argv[i + 1]);
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    CopyMarkerConfig markerConfig;
    if (custom_markers) {
        markerConfig = CopyMarkerConfig(top_left, top_right, bottom_left, bottom_right, header);
    } else {
        markerConfig = CopyMarkerConfig::getConfigById(marker_config);
    }

    bool success = create_copy(style_params, markerConfig, filename);

    return success ? 0 : 1;
}