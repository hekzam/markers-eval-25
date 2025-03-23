#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

/**
 * Generates copies of markers with configurable parameters
 *
 * @param encoded_marker_size Size of encoded markers
 * @param fiducial_marker_size Size of fiducial markers
 * @param stroke_width Width of marker stroke
 * @param marker_margin Margin around markers
 * @param nb_copies Number of copies to generate
 * @param duplex_printing 0: single-sided, 1: double-sided
 * @param marker_config Marker configuration (1-10)
 * @param grey_level Grey level (0: black, 255: white)
 * @return True if successful, false otherwise
 */
bool createCopy(int encoded_marker_size = 15, int fiducial_marker_size = 10, int stroke_width = 2,
                int marker_margin = 3, int nb_copies = 1, int duplex_printing = 0, int marker_config = 10,
                int grey_level = 100) {
    try {
        fs::create_directories("./copies");
        fs::create_directories("./output");

        for (const auto& entry : fs::directory_iterator("./copies")) {
            fs::remove_all(entry.path());
        }
        for (const auto& entry : fs::directory_iterator("./output")) {
            fs::remove_all(entry.path());
        }

        std::string doc = "template.typ";
        std::string root = ".";

        std::string params = "--input encoded-marker-size=" + std::to_string(encoded_marker_size) + " " +
                             "--input fiducial-marker-size=" + std::to_string(fiducial_marker_size) + " " +
                             "--input stroke-width=" + std::to_string(stroke_width) + " " +
                             "--input marker-margin=" + std::to_string(marker_margin) + " " +
                             "--input nb-copies=" + std::to_string(nb_copies) + " " +
                             "--input duplex-printing=" + std::to_string(duplex_printing) + " " +
                             "--input marker-config=" + std::to_string(marker_config) + " " +
                             "--input grey-level=" + std::to_string(grey_level);

        std::string compile_cmd = "typst compile --root \"" + root + "\" " + params + " \"typst/" + doc +
                                  "\" \"./copies/copy.png\" --format png";

        std::string query_atomic_boxes = "typst query --one --field value --root \"" + root + "\" " + params +
                                         " \"typst/" + doc + "\" '<atomic-boxes>' --pretty > original_boxes.json";

        std::string query_page = "typst query --one --field value --root \"" + root + "\" " + params + " \"typst/" +
                                 doc + "\" '<page>' --pretty > page.json";

        std::cout << "Executing: " << compile_cmd << std::endl;
        int compile_result = system(compile_cmd.c_str());
        if (compile_result != 0) {
            std::cerr << "Error during compilation command" << std::endl;
            return false;
        }

        std::cout << "Executing: " << query_atomic_boxes << std::endl;
        int query1_result = system(query_atomic_boxes.c_str());
        if (query1_result != 0) {
            std::cerr << "Error during query atomic boxes command" << std::endl;
            return false;
        }

        std::cout << "Executing: " << query_page << std::endl;
        int query2_result = system(query_page.c_str());
        if (query2_result != 0) {
            std::cerr << "Error during query page command" << std::endl;
            return false;
        }

        std::cout << "Copy generation completed successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

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
    int marker_config = 10;
    int grey_level = 100;

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
        } else if (arg == "--grey-level") {
            grey_level = value;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    bool success = createCopy(encoded_marker_size, fiducial_marker_size, stroke_width, marker_margin, nb_copies,
                              duplex_printing, marker_config, grey_level);

    return success ? 0 : 1;
}
