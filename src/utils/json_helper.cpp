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
        box.width = value["width"];
        box.height = value["height"];

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