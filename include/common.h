#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using json = nlohmann::json;

enum Corner { TOP_LEFT = 0x00, TOP_RIGHT = 0x01, BOTTOM_LEFT = 0x02, BOTTOM_RIGHT = 0x03, TOP_CENTER = 0x04 };

enum CornerBF { TOP_LEFT_BF = 0x01, TOP_RIGHT_BF = 0x02, BOTTOM_LEFT_BF = 0x04, BOTTOM_RIGHT_BF = 0x08 };

struct AtomicBox {
    std::string id;
    int page;
    float x;
    float y;
    float width;
    float height;
    float stroke_width;
};

struct Metadata {
    int id;
    int page;
    std::string name;
};

struct Parser {
    std::optional<cv::Mat> (*parser)(const cv::Mat& img,
#ifdef DEBUG
                                     cv::Mat debug_img,
#endif
                                     Metadata&, std::vector<cv::Point2f>&, int flag_barcode);
};

/**
 * @brief Types de parseurs disponibles pour l'analyse des marqueurs dans l'image
 */
enum class ParserType { ARUCO, CIRCLE, QRCODE, CUSTOM_MARKER, SHAPE, CENTER_MARKER_PARSER, DEFAULT_PARSER, EMPTY };

#endif