#ifndef CVS_UTILS_H
#define CVS_UTILS_H

#include <tuple>
#include <string>
#include <fstream>
#include <filesystem>

enum class CsvMode {
    APPEND,
    OVERWRITE
};

template <class... Args> class Csv {
  public:
    Csv(const std::string& filename, std::vector<std::string> headers, CsvMode mode = CsvMode::OVERWRITE) : filename_(filename) {
        bool file_exists = std::filesystem::exists(filename_);
        
        if (file_exists && mode == CsvMode::OVERWRITE) {
            std::filesystem::remove(filename_);
            file_exists = false;
        }

        csv_.open(filename_, std::ios::out | std::ios::app);
        if (!csv_.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filename_);
        }

        if (!file_exists) {
            for (size_t i = 0; i < headers.size(); ++i) {
                csv_ << headers[i];
                if (i < headers.size() - 1) {
                    csv_ << ",";
                }
            }
            csv_ << std::endl;
        }
    }

    ~Csv() {
        if (csv_.is_open()) {
            csv_.close();
        }
    }

    void add_row(const std::tuple<Args...>& data) {
        std::apply(
            [this](const auto&... args) {
                size_t i = 0;
                ((csv_ << args << (++i != sizeof...(Args) ? "," : "")), ...);
                csv_ << std::endl;
            },
            data);
    }

  private:
    std::string filename_;
    std::ofstream csv_;
};

#endif