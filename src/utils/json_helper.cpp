#include <common.h>
#include "json_helper.h"
#include "string_helper.h"

std::vector<std::shared_ptr<AtomicBox>> json_to_atomicBox(const json& content) {
    std::vector<std::shared_ptr<AtomicBox>> boxes;
    for (const auto& [key, value] : content.items()) {
        AtomicBox box;
        box.id = key;
        box.page = value["page"];
        box.x = value["x"];
        box.y = value["y"];
        if (value.contains("width")) {
            box.width = value["width"];
        } else {
            box.width = value["diameter"];
            box.x -= box.width / 2;
        }
        if (value.contains("height")) {
            box.height = value["height"];
        } else {
            box.height = value["diameter"];
            box.y -= box.height / 2;
        }

        boxes.emplace_back(std::make_shared<AtomicBox>(box));
    }
    return boxes;
}

Metadata parse_metadata(std::string content) {
    auto tokens = split(content, ",");
    Metadata metadata;
    metadata.name = tokens[0];
    metadata.page = std::strtoul(tokens[2].c_str(), nullptr, 10);
    metadata.id = std::strtoul(tokens[1].c_str(), nullptr, 10);

    return metadata;
}