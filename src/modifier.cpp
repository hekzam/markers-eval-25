#include <common.h>

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }
    std::string image_path = argv[1];
    cv::Mat img = cv::imread(image_path);
    return 0;
}
