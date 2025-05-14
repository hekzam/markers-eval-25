#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <cstdlib>

#include <create_copy.h>

namespace fs = std::filesystem;

namespace {
const std::unordered_map<MarkerType, std::string> markerTypeToString = {
    { MarkerType::QR_CODE, "qrcode" },        { MarkerType::MICRO_QR_CODE, "microqr" },
    { MarkerType::DATAMATRIX, "datamatrix" }, { MarkerType::AZTEC, "aztec" },
    { MarkerType::PDF417, "pdf417" },         { MarkerType::RMQR, "rmqr" },
    { MarkerType::BARCODE, "code128" },       { MarkerType::CIRCLE, "circle" },
    { MarkerType::SQUARE, "square" },         { MarkerType::ARUCO, "aruco" },
    { MarkerType::QR_EYE, "qreye" },          { MarkerType::CROSS, "cross" },
    { MarkerType::CUSTOM, "custom" },         { MarkerType::NONE, "" }
};

const std::unordered_map<std::string, MarkerType> stringToMarkerType = { { "qrcode", MarkerType::QR_CODE },
                                                                         { "microqr", MarkerType::MICRO_QR_CODE },
                                                                         { "datamatrix", MarkerType::DATAMATRIX },
                                                                         { "aztec", MarkerType::AZTEC },
                                                                         { "pdf417", MarkerType::PDF417 },
                                                                         { "rmqr", MarkerType::RMQR },
                                                                         { "code128", MarkerType::BARCODE },
                                                                         { "circle", MarkerType::CIRCLE },
                                                                         { "square", MarkerType::SQUARE },
                                                                         { "aruco", MarkerType::ARUCO },
                                                                         { "qreye", MarkerType::QR_EYE },
                                                                         { "cross", MarkerType::CROSS },
                                                                         { "custom", MarkerType::CUSTOM },
                                                                         { "none", MarkerType::NONE },
                                                                         { "", MarkerType::NONE } };
} // namespace

std::string toString(MarkerType type) {
    auto it = markerTypeToString.find(type);
    if (it != markerTypeToString.end()) {
        return it->second;
    }
    return "";
}

std::string Marker::toString() const {
    if (type == MarkerType::NONE) {
        return "none";
    }

    std::string result = ::toString(type);

    if (outlined) {
        result += "-outlined";
    }
    if (encoded) {
        result += "-encoded";
    }
    return result;
}

MarkerType markerTypeFromString(const std::string& typeStr) {
    auto it = stringToMarkerType.find(typeStr);
    if (it != stringToMarkerType.end()) {
        return it->second;
    }
    return MarkerType::NONE;
}

Marker Marker::parseMarker(const std::string& spec) {
    if (spec.empty() || spec == "none") {
        return Marker();
    }

    std::string type = spec;
    bool encoded = false;
    bool outlined = false;

    size_t encodedPos = spec.find("-encoded");
    if (encodedPos != std::string::npos) {
        encoded = true;
        type = spec.substr(0, encodedPos);
    } else {
        encodedPos = spec.find(":encoded");
        if (encodedPos != std::string::npos) {
            encoded = true;
            type = spec.substr(0, encodedPos);
        }
    }

    size_t outlinedPos = spec.find("-outlined");
    if (outlinedPos != std::string::npos) {
        outlined = true;
        if (encodedPos != std::string::npos && outlinedPos < encodedPos) {
            type = spec.substr(0, outlinedPos);
        } else if (encodedPos == std::string::npos) {
            type = spec.substr(0, outlinedPos);
        }
    } else {
        outlinedPos = spec.find(":outlined");
        if (outlinedPos != std::string::npos) {
            outlined = true;
            if (encodedPos != std::string::npos && outlinedPos < encodedPos) {
                type = spec.substr(0, outlinedPos);
            } else if (encodedPos == std::string::npos) {
                type = spec.substr(0, outlinedPos);
            }
        }
    }

    return Marker(markerTypeFromString(type), encoded, outlined);
}

std::string CopyMarkerConfig::toString() const {
    return "(" + top_left.toString() + "," + top_right.toString() + "," + bottom_left.toString() + "," +
           bottom_right.toString() + "," + header.toString() + ")";
}

int CopyMarkerConfig::fromString(const std::string& str, CopyMarkerConfig& config) {
    if (str.size() < 2 || str[0] != '(' || str[str.size() - 1] != ')') {
        return 1;
    }

    std::string content = str.substr(1, str.size() - 2);

    std::vector<std::string> markerStrs;
    size_t pos = 0, found;
    while ((found = content.find(',', pos)) != std::string::npos) {
        markerStrs.push_back(content.substr(pos, found - pos));
        pos = found + 1;
    }
    markerStrs.push_back(content.substr(pos));

    if (markerStrs.size() != 5) {
        return 1;
    }

    config.top_left = Marker::parseMarker(markerStrs[0]);
    config.top_right = Marker::parseMarker(markerStrs[1]);
    config.bottom_left = Marker::parseMarker(markerStrs[2]);
    config.bottom_right = Marker::parseMarker(markerStrs[3]);
    config.header = Marker::parseMarker(markerStrs[4]);

    return 0;
}

std::string getOutputRedirection() {
#ifdef _WIN32
    return " 2> NUL";
#else
    return " 2> /dev/null";
#endif
}

bool create_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                 const std::string& filename, bool verbose) {

    fs::create_directories("./copies");

    std::string doc = "template.typ";
    std::string root = "..";
    std::string redirect = verbose ? "" : getOutputRedirection();

    std::string params = "--input encoded-marker-size=" + std::to_string(style_params.encoded_marker_size) + " " +
                         "--input unencoded-marker-size=" + std::to_string(style_params.unencoded_marker_size) + " " +
                         "--input header-marker-size=" + std::to_string(style_params.header_marker_size) + " " +
                         "--input stroke-width=" + std::to_string(style_params.stroke_width) + " " +
                         "--input marker-margin=" + std::to_string(style_params.marker_margin) + " " +
                         "--input grey-level=" + std::to_string(style_params.grey_level) + " " +
                         "--input generating-content=" + (style_params.generating_content ? "1" : "0") + " " +
                         "--input marker-types=" + "\"" + marker_config.toString() + "\"";

    std::string compile_cmd = "typst compile --root \"" + root + "\" " + params + " \"" + root + "/typst/" + doc +
                              "\" \"./copies/" + filename + ".png\" --format png --ppi " +
                              std::to_string(style_params.dpi) + redirect;

    std::string query_atomic_boxes = "typst query --one --field value --root \"" + root + "\" " + params + " \"" +
                                     root + "/typst/" + doc + "\" '<atomic-boxes>' --pretty > original_boxes.json" +
                                     redirect;

    std::string query_page = "typst query --one --field value --root \"" + root + "\" " + params + " \"" + root +
                             "/typst/" + doc + "\" '<page>' --pretty > page.json" + redirect;

    int compile_result = system(compile_cmd.c_str());
    if (compile_result != 0) {
        std::cerr << "Error during compilation command" << std::endl;
        return false;
    }

    int query1_result = system(query_atomic_boxes.c_str());
    if (query1_result != 0) {
        std::cerr << "Error during query atomic boxes command" << std::endl;
        return false;
    }

    // int query2_result = system(query_page.c_str());
    // if (query2_result != 0) {
    //     std::cerr << "Error during query page command" << std::endl;
    //     return false;
    // }

    std::cout << "Copy generation completed successfully" << std::endl;
    return true;
}