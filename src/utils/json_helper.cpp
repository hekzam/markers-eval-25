#include <common.h>
#include "json_helper.h"

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