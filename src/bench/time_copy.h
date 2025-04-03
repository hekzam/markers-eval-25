#ifndef TIME_COPY_H
#define TIME_COPY_H

void run_benchmark(std::unordered_map<std::string, Config> config);
extern std::unordered_map<std::string, Config> default_config_time_copy;
bool constraint(std::unordered_map<std::string, Config> config);

#endif // TIME_COPY_H