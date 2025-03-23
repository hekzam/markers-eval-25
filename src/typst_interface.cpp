#include <iostream>
#include <string>
#include <cstdlib>
#include "external-tools/create_copy.h"

bool isValidMarkerConfig(int config) {
    return config >= QR_ALL_CORNERS && config <= SQUARE_OUTLINES_WITH_QR_BR;
}

extern bool create_copy(int encoded_marker_size, int fiducial_marker_size, int stroke_width, int marker_margin,
                        int nb_copies, int duplex_printing, int marker_config, int grey_level);

/**
 * Main function for marker generation utility
 * Accepts command-line arguments to customize marker generation:
 *
 * Usage: program [options]
 * Options:
 *   --encoded-size N      : Size of encoded markers (default: 15)
 *   --fiducial-size N     : Size of fiducial markers (default: 10)
 *   --stroke-width N      : Width of marker stroke (default: 2)
 *   --margin N            : Margin around markers (default: 3)
 *   --copies N            : Number of copies to generate (default: 1)
 *   --duplex N            : Duplex printing mode (0: single-sided, 1: double-sided) (default: 0)
 *   --config N            : Marker configuration (1-10) (default: 10)
 *   --grey-level N        : Grey level (0: black, 255: white) (default: 100)
 */
int main(int argc, char* argv[]) {
    int encoded_marker_size = 15;
    int fiducial_marker_size = 10;
    int stroke_width = 2;
    int marker_margin = 3;
    int nb_copies = 1;
    int duplex_printing = 0;
    int marker_config = CIRCLES_WITH_QR_BR;
    int grey_level = 255;

    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];

        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument: " << arg << std::endl;
            return 1;
        }

        int value = std::atoi(argv[i + 1]);

        if (arg == "--encoded-size") {
            encoded_marker_size = value;
        } else if (arg == "--fiducial-size") {
            fiducial_marker_size = value;
        } else if (arg == "--stroke-width") {
            stroke_width = value;
        } else if (arg == "--margin") {
            marker_margin = value;
        } else if (arg == "--copies") {
            nb_copies = value;
        } else if (arg == "--duplex") {
            duplex_printing = value;
        } else if (arg == "--config") {
            marker_config = value;
            if (!isValidMarkerConfig(marker_config)) {
                std::cerr << "Invalid marker configuration: " << value << " (valid range: 1-10)" << std::endl;
                return 1;
            }
        } else if (arg == "--grey-level") {
            grey_level = value;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    bool success = create_copy(encoded_marker_size, fiducial_marker_size, stroke_width, marker_margin, nb_copies,
                               duplex_printing, marker_config, grey_level);

    return success ? 0 : 1;
}
