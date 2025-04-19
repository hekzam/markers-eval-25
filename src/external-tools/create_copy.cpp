#include <iostream>
#include <map>
#include <filesystem>
#include <cstdlib>

#include <create_copy.h>

namespace fs = std::filesystem;

const std::vector<MarkerConfigInfo> marker_configs = {
    { QR_ALL_CORNERS, "QR codes in all corners" },
    { QR_BOTTOM_RIGHT_ONLY, "QR code only in bottom-right corner" },
    { CIRCLES_WITH_QR_BR, "Circles in first three corners, QR code in bottom-right" },
    { TOP_CIRCLES_QR_BR, "Circles on top, nothing in bottom-left, QR code in bottom-right" },
    { CUSTOM_SVG_WITH_QR_BR, "Custom SVG markers in three corners, QR code in bottom-right" },
    { ARUCO_WITH_QR_BR, "ArUco markers, QR code in bottom-right" },
    { TWO_ARUCO_WITH_QR_BR, "Two ArUco markers, nothing in bottom-left, QR code in bottom-right" },
    { CIRCLE_OUTLINES_WITH_QR_BR, "Circle outlines in first three corners, QR code in bottom-right" },
    { SQUARES_WITH_QR_BR, "Squares in first three corners, QR code in bottom-right" },
    { SQUARE_OUTLINES_WITH_QR_BR, "Square outlines in first three corners, QR code in bottom-right" }
};

std::string getOutputRedirection() {
#ifdef _WIN32
    return " 2> NUL";
#else
    return " 2> /dev/null";
#endif
}

bool create_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                 const std::string& filename) {

    fs::create_directories("./copies");

    std::string doc = "template.typ";
    std::string root = ".";
    std::string redirect = getOutputRedirection();

    std::string params = "--input encoded-marker-size=" + std::to_string(style_params.encoded_marker_size) + " " +
                         "--input fiducial-marker-size=" + std::to_string(style_params.fiducial_marker_size) + " " +
                         "--input header-marker-size=" + std::to_string(style_params.header_marker_size) + " " +
                         "--input stroke-width=" + std::to_string(style_params.stroke_width) + " " +
                         "--input marker-margin=" + std::to_string(style_params.marker_margin) + " " +
                         "--input grey-level=" + std::to_string(style_params.grey_level) + " " +
                         "--input marker-types=" + "\"" + marker_config.toString() + "\"";

    std::string compile_cmd = "typst compile --root \"" + root + "\" " + params + " \"typst/" + doc + "\" \"./copies/" +
                              filename + ".png\" --format png --ppi " + std::to_string(style_params.dpi) + redirect;

    std::string query_atomic_boxes = "typst query --one --field value --root \"" + root + "\" " + params + " \"typst/" +
                                     doc + "\" '<atomic-boxes>' --pretty > original_boxes.json" + redirect;

    std::string query_page = "typst query --one --field value --root \"" + root + "\" " + params + " \"typst/" + doc +
                             "\" '<page>' --pretty > page.json" + redirect;

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

    int query2_result = system(query_page.c_str());
    if (query2_result != 0) {
        std::cerr << "Error during query page command" << std::endl;
        return false;
    }

    std::cout << "Copy generation completed successfully" << std::endl;
    return true;
}