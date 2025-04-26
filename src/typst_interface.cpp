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
    std::string filename = "copy";

    Marker top_left, top_right, bottom_left, bottom_right, header;
    top_left = Marker(MarkerType::QR_CODE, true);
    top_right = Marker(MarkerType::QR_CODE, true);
    bottom_left = Marker(MarkerType::QR_CODE, true);
    bottom_right = Marker(MarkerType::QR_CODE, true);
    header = Marker();

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
        } else if (arg == "--generating-content") {
            style_params.generating_content = (std::string(argv[i + 1]) == "1" || std::string(argv[i + 1]) == "true");
        } else if (arg == "--tl") {
            top_left = Marker::parseMarker(argv[i + 1]);
        } else if (arg == "--tr") {
            top_right = Marker::parseMarker(argv[i + 1]);
        } else if (arg == "--bl") {
            bottom_left = Marker::parseMarker(argv[i + 1]);
        } else if (arg == "--br") {
            bottom_right = Marker::parseMarker(argv[i + 1]);
        } else if (arg == "--header") {
            header = Marker::parseMarker(argv[i + 1]);
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

    CopyMarkerConfig markerConfig = CopyMarkerConfig(top_left, top_right, bottom_left, bottom_right, header);

    bool success = create_copy(style_params, markerConfig, filename);

    return success ? 0 : 1;
}