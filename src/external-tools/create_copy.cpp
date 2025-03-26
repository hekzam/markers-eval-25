#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <cstdlib>

#include <create_copy.h>

namespace fs = std::filesystem;

bool create_copy(int encoded_marker_size, int fiducial_marker_size, int stroke_width, int marker_margin, int nb_copies,
                 int duplex_printing, int marker_config, int grey_level, int header_marker) {

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
                         "--input grey-level=" + std::to_string(grey_level) + " " +
                         "--input header-marker=" + std::to_string(header_marker);

    std::string compile_cmd =
        "typst compile --root \"" + root + "\" " + params + " \"typst/" + doc + "\" \"./copies/copy.png\" --format png";

    std::string query_atomic_boxes = "typst query --one --field value --root \"" + root + "\" " + params + " \"typst/" +
                                     doc + "\" '<atomic-boxes>' --pretty > original_boxes.json";

    std::string query_page = "typst query --one --field value --root \"" + root + "\" " + params + " \"typst/" + doc +
                             "\" '<page>' --pretty > page.json";

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
}
