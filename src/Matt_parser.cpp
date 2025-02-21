#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <poppler-rectangle.h>
#include <iostream>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <cmath>

using json = nlohmann::json;

// Seuils pour filtrer les cercles dans les coins
const double TOLERANCE = 30.0;
const double EXPECTED_DIAMETER = 40.5;
const std::vector<cv::Point2f> expected_corners = {
    {153, 153}, {1159, 3384}, {1206, 3383}, {1184, 3379}
};

bool is_corner_circle(double x, double y, double radius) {
    for (const auto& corner : expected_corners) {
        if (std::abs(x - corner.x) < TOLERANCE && std::abs(y - corner.y) < TOLERANCE && std::abs(radius * 2 - EXPECTED_DIAMETER) < 10.0) {
            return true;
        }
    }
    return false;
}

json extract_circles(const cv::Mat &image, int page_number, double mm_to_pixel) {
    json circles = json::object();
    std::vector<cv::Vec3f> detected_circles;
    
    cv::Mat blurred;
    cv::GaussianBlur(image, blurred, cv::Size(9, 9), 2);
    cv::HoughCircles(blurred, detected_circles, cv::HOUGH_GRADIENT, 1, image.rows / 8, 100, 30, 10, 50);
    
    std::cout << "Nombre de cercles détectés : " << detected_circles.size() << std::endl;
    
    std::vector<std::tuple<double, double, double, double>> closest_circles(4, std::make_tuple(std::numeric_limits<double>::max(), 0.0, 0.0, 0.0));
    
    std::vector<cv::Point2f> target_points = {
        {12.642568545329247, 12.642568545329247},
        {196.64256854532925, 12.642568545329247},
        {12.642568545329247, 282.5357574342181},
        {196.64256854532925, 282.5357574342181}
    };
    
    for (const auto& c : detected_circles) {
        double x = c[0];
        double y = c[1];
        double radius = c[2];
        for (size_t i = 0; i < target_points.size(); ++i) {
            double distance = std::hypot(x - target_points[i].x, y - target_points[i].y);
            std::cout << "Cercle détecté : (" << x << ", " << y << ") | Distance au point cible " << i << " : " << distance << std::endl;
            if (distance < std::get<0>(closest_circles[i])) {
                closest_circles[i] = std::make_tuple(distance, x, y, radius);
            }
        }
    }
    
    for (size_t i = 0; i < closest_circles.size(); ++i) {
        if (std::get<0>(closest_circles[i]) < std::numeric_limits<double>::max()) {
            double x = std::get<1>(closest_circles[i]);
            double y = std::get<2>(closest_circles[i]);
            double radius = std::get<3>(closest_circles[i]);
            json circle_metadata = {
                {"page", page_number},
                {"x", x / mm_to_pixel},
                {"y", y / mm_to_pixel},
                {"diameter", 2 * radius / mm_to_pixel},
                {"stroke-width", 0.0},
                {"stroke-color", "#000000"},
                {"fill-color", "#000000"}
            };
            std::string key;
            switch (i) {
                case 0: key = "marker circle tl page" + std::to_string(page_number); break;
                case 1: key = "marker circle tr page" + std::to_string(page_number); break;
                case 2: key = "marker circle bl page" + std::to_string(page_number); break;
                case 3: key = "marker circle br page" + std::to_string(page_number); break;
            }
            circles[key] = circle_metadata;
        }
    }
    
    return circles;
}

json extract_rectangles(const cv::Mat &image, int page_number, double mm_to_pixel) {
    json rectangles = json::object();
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat binary;
    cv::adaptiveThreshold(image, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 11, 2);
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    int count = 1;
    for (const auto &contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        double aspect_ratio = (double)rect.width / (double)rect.height;
        if (rect.width > 50 && rect.height > 50 && rect.width < 500 && rect.height < 500 && aspect_ratio > 0.5 && aspect_ratio < 2.0) {
            json rect_metadata = {
                {"page", page_number},
                {"x", rect.x / mm_to_pixel},
                {"y", rect.y / mm_to_pixel},
                {"width", rect.width / mm_to_pixel},
                {"height", rect.height / mm_to_pixel},
                {"stroke-width", 0.25},
                {"stroke-color", "#000000"},
                {"fill-color", "#0000ff"}
            };
            rectangles["b" + std::to_string(count)] = rect_metadata;
            count++;
        }
    }
    return rectangles;
}

void extract_metadata(const std::string &pdf_path, const std::string &output_path) {
    poppler::document *doc = poppler::document::load_from_file(pdf_path);
    if (!doc) {
        std::cerr << "Error: Could not open PDF file: " << pdf_path << std::endl;
        return;
    }
    json metadata = json::object();
    for (int i = 0; i < doc->pages(); ++i) {
        poppler::page *p = doc->create_page(i);
        if (!p) continue;
        
        double page_width = p->page_rect().width();
        double page_height = p->page_rect().height();
        double mm_to_pixel = 300.0 / 25.4;
        
        json page_metadata = {
            {"page", i + 1},
            {"width", page_width},
            {"height", page_height}
        };
        
        poppler::page_renderer renderer;
        renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
        renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
        poppler::image p_image = renderer.render_page(p, 300, 300, -1, -1, -1, -1);
        cv::Mat image(p_image.height(), p_image.width(), CV_8UC4, (void*)p_image.data(), p_image.bytes_per_row());
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(image, image);
        
        json circles = extract_circles(image, i + 1, mm_to_pixel);
        json rectangles = extract_rectangles(image, i + 1, mm_to_pixel);
        
        for (auto &circle : circles.items()) {
            metadata[circle.key()] = circle.value();
        }
        for (auto &rect : rectangles.items()) {
            metadata[rect.key()] = rect.value();
        }
        delete p;
    }
    delete doc;
    
    std::ofstream output_file(output_path);
    output_file << metadata.dump(4);
    output_file.close();
}

int main() {
    std::filesystem::create_directory("meta");
    for (int i = 1; i <= 10; ++i) {
        std::string pdf_path = "/home/asulf/pdf_pourrisseur/copie_pourrie/output" + std::to_string(i) + ".pdf";
        std::string output_path = "meta/meta" + std::to_string(i) + ".json";
        extract_metadata(pdf_path, output_path);
    }
    return 0;
}
