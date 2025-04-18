#include <iostream>
#include <string>
#include <cstdlib>
#include "external-tools/create_copy.h"

bool isValidMarkerConfig(int config) {
    return config >= QR_ALL_CORNERS && config <= SQUARE_OUTLINES_WITH_QR_BR;
}

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
    
    return Marker(type, encoded, outlined);
}

int main(int argc, char* argv[]) {
    CopyStyleParams style_params;
    int duplex_printing = 0;
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
        } else if (arg == "--fiducial-size") {
            style_params.fiducial_marker_size = std::atoi(argv[i + 1]);
        } else if (arg == "--stroke-width") {
            style_params.stroke_width = std::atoi(argv[i + 1]);
        } else if (arg == "--margin") {
            style_params.marker_margin = std::atoi(argv[i + 1]);
        } else if (arg == "--copies") {
            style_params.nb_copies = std::atoi(argv[i + 1]);
        } else if (arg == "--duplex") {
            duplex_printing = std::atoi(argv[i + 1]);
        } else if (arg == "--config") {
            marker_config = std::atoi(argv[i + 1]);
            if (!isValidMarkerConfig(marker_config)) {
                std::cerr << "Invalid marker configuration: " << argv[i + 1] << " (valid range: 1-10)" << std::endl;
                return 1;
            }
        } else if (arg == "--top-left") {
            top_left = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--top-right") {
            top_right = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--bottom-left") {
            bottom_left = parseMarker(argv[i + 1]);
            custom_markers = true;
        } else if (arg == "--bottom-right") {
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

    bool success = create_copy(style_params, duplex_printing, markerConfig, filename);

    return success ? 0 : 1;
}
