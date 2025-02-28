#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum Corner { TOP_LEFT = 0x00, TOP_RIGHT = 0x01, BOTTOM_LEFT = 0x02, BOTTOM_RIGHT = 0x03 };

struct AtomicBox {
    std::string id;
    int page;
    float x;
    float y;
    float width;
    float height;
};

#endif