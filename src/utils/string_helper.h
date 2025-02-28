#ifndef STRING_HELPER_H
#define STRING_HELPER_H
#include <vector>
#include <string>

std::vector<std::string> split(std::string s, std::string delimiter);
bool starts_with(std::string s, std::string prefix);

#endif