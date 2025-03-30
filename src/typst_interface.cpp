#include <iostream>
#include <string>
#include <cstdlib>
#include "external-tools/create_copy.h"

bool isValidMarkerConfig(int config) {
    return config >= QR_ALL_CORNERS && config <= SQUARE_OUTLINES_WITH_QR_BR;
}

extern bool create_copy(int encoded_marker_size, int fiducial_marker_size, int stroke_width, int marker_margin,
                        int nb_copies, int duplex_printing, int marker_config, int grey_level, int header_marker,
                        const std::string& filename);

int main(int argc, char* argv[]) {
    int encoded_marker_size = 15;
    int fiducial_marker_size = 3;
    int stroke_width = 2;
    int marker_margin = 3;
    int nb_copies = 1;
    int duplex_printing = 0;
    int marker_config = CIRCLES_WITH_QR_BR;
    int grey_level = 0;
    int header_marker = 1;
    std::string filename = "copy";

    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];

        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument: " << arg << std::endl;
            return 1;
        }

        if (arg == "--encoded-size") {
            encoded_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--fiducial-size") {
            fiducial_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--stroke-width") {
            stroke_width = std::atoi(argv[i + 1]);
        } else if (arg == "--margin") {
            marker_margin = std::atoi(argv[i + 1]);
        } else if (arg == "--copies") {
            nb_copies = std::atoi(argv[i + 1]);
        } else if (arg == "--duplex") {
            duplex_printing = std::atoi(argv[i + 1]);
        } else if (arg == "--config") {
            marker_config = std::atoi(argv[i + 1]);
            if (!isValidMarkerConfig(marker_config)) {
                std::cerr << "Invalid marker configuration: " << argv[i + 1] << " (valid range: 1-10)" << std::endl;
                return 1;
            }
        } else if (arg == "--grey-level") {
            grey_level = std::atoi(argv[i + 1]);
        } else if (arg == "--header-marker") {
            header_marker = std::atoi(argv[i + 1]);
        } else if (arg == "--filename") {
            filename = argv[i + 1];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    bool success = create_copy(encoded_marker_size, fiducial_marker_size, stroke_width, marker_margin, nb_copies,
                               duplex_printing, marker_config, grey_level, header_marker, filename);

    return success ? 0 : 1;
}
