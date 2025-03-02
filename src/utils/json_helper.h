#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <common.h>
#include <vector>
#include <memory>

std::vector<std::shared_ptr<AtomicBox>> json_to_atomicBox(const json& content);
Metadata parse_metadata(std::string content);

#endif // JSON_HELPER_H
