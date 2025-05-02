#ifndef CVS_UTILS_H
#define CVS_UTILS_H

#include <tuple>
#include <string>
#include <fstream>

template <class... Args> class Csv {
  public:
    Csv(const std::string& filename, std::vector<std::string> headers) : filename_(filename) {
        csv_.open(filename_, std::ios::out | std::ios::app);
        if (!csv_.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filename_);
        }

        // Write header
        for (const auto& header : headers) {
            csv_ << header << ",";
        }
        csv_ << std::endl;
    }

    ~Csv() {
        if (csv_.is_open()) {
            csv_.close();
        }
    }

    void add_row(const std::tuple<Args...>& data) {
        std::apply(
            [this](const auto&... args) {
                ((csv_ << args << ","), ...);
                csv_ << std::endl;
            },
            data);
    }

  private:
    std::string filename_;
    std::ofstream csv_;
};

#endif