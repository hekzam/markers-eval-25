#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <filesystem>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <ZXing/ReadBarcode.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum Corner {
  TOP_LEFT     = 0x00
, TOP_RIGHT    = 0x01
, BOTTOM_LEFT  = 0x02
, BOTTOM_RIGHT = 0x03
};

enum CornerBF {
  TOP_LEFT_BF     = 0x01
, TOP_RIGHT_BF    = 0x02
, BOTTOM_LEFT_BF  = 0x04
, BOTTOM_RIGHT_BF = 0x08
};

struct DetectedBarcode {
  std::string content;
  std::vector<cv::Point2f> bounding_box;
};

struct AtomicBox {
  std::string id;
  int page;
  float x;
  float y;
  float width;
  float height;
};

void parse_atomic_boxes(const json & content, std::vector<AtomicBox> & boxes) {
  for (const auto & [key, value] : content.items()) {
    AtomicBox box;
    box.id = key;
    box.page = value["page"];
    box.x = value["x"];
    box.y = value["y"];
    box.width = value["width"];
    box.height = value["height"];

    boxes.emplace_back(box);
  }
}

void differentiate_atomic_boxes(
  std::vector<AtomicBox> & boxes,
  std::vector<AtomicBox*> & markers,
  std::vector<AtomicBox*> & corner_markers,
  std::vector<std::vector<AtomicBox*>> & user_boxes_per_page
) {
  markers.clear();
  corner_markers.resize(4);
  user_boxes_per_page.clear();

  if (boxes.empty())
    return;
  int max_page = 1;

  for (const auto & box : boxes) {
    max_page = std::max(max_page, box.page);
  }
  user_boxes_per_page.resize(max_page);

  for (AtomicBox & box : boxes) {
    if (box.id.find("marker barcode ") == 0) {
      markers.emplace_back(&box);
    } else {
      user_boxes_per_page.at(box.page - 1).emplace_back(&box);
    }
  }

  int corner_mask = 0;
  for (auto * box : markers) {
    int corner = -1;
    if (box->id == "marker barcode tl page1")
      corner = TOP_LEFT;
    else if (box->id == "marker barcode tr page1")
      corner = TOP_RIGHT;
    else if (box->id == "marker barcode bl page1")
      corner = BOTTOM_LEFT;
    else if (box->id == "marker barcode br page1")
      corner = BOTTOM_RIGHT;

    if (corner != -1) {
      corner_markers[corner] = box;
      corner_mask |= (1 << corner);
    }
  }

  if (corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
    throw std::invalid_argument("some corner markers are missing in the atomic box JSON description");
}

// 2d coordinate transformation (scale), assuming the two are (0,0)-based
cv::Point2f coord_scale(
  const cv::Point2f & src_coord,
  const cv::Point2f & src_img_size,
  const cv::Point2f & dst_img_size
) {
  return cv::Point2f {
    (src_coord.x / src_img_size.x) * dst_img_size.x,
    (src_coord.y / src_img_size.y) * dst_img_size.y,
  };
}

void compute_dst_corner_points(
  const std::vector<AtomicBox*> & corner_markers,
  const cv::Point2f & src_img_size,
  const cv::Point2f & dst_img_size,
  std::vector<cv::Point2f> & corner_points
) {
  corner_points.resize(4);
  for (int corner = 0; corner < 4; ++corner) {
    auto * box = corner_markers[corner];
    const std::vector<cv::Point2f> bounding_box = {
      cv::Point2f{box->x, box->y},
      cv::Point2f{box->x + box->width, box->y},
      cv::Point2f{box->x + box->width, box->y + box->height},
      cv::Point2f{box->x, box->y + box->height}
    };
    cv::Mat mean_mat;
    cv::reduce(bounding_box, mean_mat, 1, cv::REDUCE_AVG);
    cv::Point2f mean_point{mean_mat.at<float>(0,0), mean_mat.at<float>(0,1)};
    //printf("corner[%d] mean point: (%f, %f)\n", corner, mean_point.x, mean_point.y);

    corner_points[corner] = coord_scale(mean_point, src_img_size, dst_img_size);
  }
}

void get_affine_transform(
  int found_corner_mask,
  const std::vector<cv::Point2f> & expected_corner_points,
  const std::vector<cv::Point2f> & found_corner_points,
  cv::Mat & affine_transform
) {
  int nb_found = 0;
  std::vector<cv::Point2f> src, dst;
  src.reserve(3);
  dst.reserve(3);

  for (int corner = 0; corner < 4; ++corner) {
    if ((1 << corner) & found_corner_mask) {
      src.emplace_back(found_corner_points[corner]);
      dst.emplace_back(expected_corner_points[corner]);

      nb_found += 1;
      if (nb_found >= 3)
        break;
    }
  }

  if (nb_found != 3)
    throw std::invalid_argument("only " + std::to_string(nb_found) + " corners were found (3 or more required)");

  /*for (int i = 0; i < 3; ++i) {
    printf("src[%d]: (%f, %f)\n", i, src[i].x, src[i].y);
    printf("dst[%d]: (%f, %f)\n", i, dst[i].x, dst[i].y);
  }*/
  affine_transform = cv::getAffineTransform(src, dst);
}

int identify_corner_barcodes(
  std::vector<DetectedBarcode> & barcodes,
  const std::string & content_hash,
  std::vector<cv::Point2f> & corner_points,
  std::vector<DetectedBarcode*> & corner_barcodes
) {
  corner_points.resize(4);
  corner_barcodes.resize(4);
  int found_mask = 0x00;

  for (auto & barcode : barcodes) {
    // should contain "hzXY" with XY in {tr, tr, br} or a content hash longer than 4
    if (barcode.content.size() < 4)
      continue;

    const char * s = barcode.content.c_str();
    int pos_found = 0;

    uint16_t hz = (s[0] << 8) + s[1];
    if (hz == ('h' << 8) + 'z') {
      // content starts with "hz"
      uint16_t xy = (s[2] << 8) + s[3];
      switch(xy) {
        case ('t' << 8) + 'l':
          pos_found = TOP_LEFT;
          break;
        case ('t' << 8) + 'r':
          pos_found = TOP_RIGHT;
          break;
        case ('b' << 8) + 'l':
          pos_found = BOTTOM_LEFT;
          break;
        case ('b' << 8) + 'r': {
          if (strstr(s, content_hash.c_str()) == NULL)
            continue;
          pos_found = BOTTOM_RIGHT;
        } break;
        default:
          continue;
      }

      cv::Mat mean_mat;
      cv::reduce(barcode.bounding_box, mean_mat, 1, cv::REDUCE_AVG);
      corner_points[pos_found] = cv::Point2f(mean_mat.at<float>(0,0), mean_mat.at<float>(0,1));
      corner_barcodes[pos_found] = &barcode;
      int pos_found_bf = 1 << pos_found;
      //printf("found pos=%d -> bf=%d\n", pos_found, pos_found_bf);
      found_mask |= pos_found_bf;
    }
  }

  return found_mask;
}

void detect_barcodes(cv::Mat img, std::vector<DetectedBarcode> & barcodes) {
  barcodes.clear();

  if (img.type() != CV_8U)
    throw std::invalid_argument("img has type != CV_8U while it should contain luminance information on 8-bit unsigned integers");

  if (img.cols < 2 || img.rows < 2)
    return;

  auto iv = ZXing::ImageView(reinterpret_cast<const uint8_t*>(img.ptr()), img.cols, img.rows, ZXing::ImageFormat::Lum);
  auto z_barcodes = ZXing::ReadBarcodes(iv);

  for (const auto & b : z_barcodes) {
    DetectedBarcode barcode;
    barcode.content = b.text();

    std::vector<cv::Point> corners;
    for (int j = 0; j < 4; ++j) {
      const auto & p = b.position()[j];
      barcode.bounding_box.emplace_back(cv::Point2f(p.x, p.y));
    }
    barcodes.emplace_back(barcode);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES IMAGE...\n");
    return 1;
  }

  // create output directory if needed
  std::filesystem::path output_dir{argv[1]};
  std::filesystem::create_directories(output_dir);

  std::filesystem::path subimg_output_dir = output_dir.string() + std::string("/subimg");
  std::filesystem::create_directories(subimg_output_dir);

  // read atomic boxes info
  std::ifstream atomic_boxes_file(argv[2]);
  if (!atomic_boxes_file.is_open()) {
    fprintf(stderr, "could not open file '%s'\n", argv[2]);
    return 1;
  }
  json atomic_boxes_json;
  try {
    atomic_boxes_json = json::parse(atomic_boxes_file);
  } catch (const json::exception & e) {
    fprintf(stderr, "could not json parse file '%s': %s", argv[2], e.what());
    return 1;
  }
  //printf("atomic_boxes: %s\n", atomic_boxes_json.dump(2).c_str());

  const std::string expected_content_hash = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=";

  std::vector<AtomicBox> atomic_boxes;
  std::vector<AtomicBox*> markers;
  std::vector<AtomicBox*> corner_markers;
  std::vector<std::vector<AtomicBox*>> user_boxes_per_page;
  parse_atomic_boxes(atomic_boxes_json, atomic_boxes);
  differentiate_atomic_boxes(atomic_boxes, markers, corner_markers, user_boxes_per_page);

  /*printf("there are %lu boxes\n", atomic_boxes.size());
  for (auto * marker : markers) {
    printf("marker: %s %d\n", marker->id.c_str(), marker->page);
  }
  for (auto * marker : corner_markers) {
    printf("corner marker: %s %d\n", marker->id.c_str(), marker->page);
  }

  for (unsigned page = 0; page < user_boxes_per_page.size(); ++page) {
    printf("user boxes of page %u\n", page+1);
    for (auto * box : user_boxes_per_page[page]) {
      printf("  box: %s %d\n", box->id.c_str(), box->page);
    }
  }*/

  const cv::Point2f src_img_size{210, 297}; // TODO: do not assume A4

  for (int i = 3; i < argc; ++i) {
    cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
    const cv::Point2f dst_img_size(img.cols, img.rows);
    // TODO: use min and max for 90 ° rotate if needed
    //printf("dst_img_size: (%f, %f)\n", dst_img_size.x, dst_img_size.y);

    std::vector<cv::Point2f> dst_corner_points;
    compute_dst_corner_points(corner_markers, src_img_size, dst_img_size, dst_corner_points);

    std::vector<DetectedBarcode> barcodes;
    detect_barcodes(img, barcodes);

    std::vector<cv::Point2f> corner_points;
    std::vector<DetectedBarcode*> corner_barcodes;
    int found_corner_mask = identify_corner_barcodes(barcodes, expected_content_hash, corner_points, corner_barcodes);

    // TODO: fix ugly code to read copy number and page number. assumes "hzbl,COPYNUMBER,PAGENUMBER"
    const char * bl_qrcode_str = corner_barcodes[BOTTOM_LEFT]->content.c_str();
    char * parse_ptr = nullptr;
    int copy = strtol(bl_qrcode_str + 5, &parse_ptr, 10);
    int page = strtol(parse_ptr + 1, NULL, 10);

    cv::Mat affine_transform;
    get_affine_transform(found_corner_mask, dst_corner_points, corner_points, affine_transform);

    cv::Mat calibrated_img = img;
    warpAffine(img, calibrated_img, affine_transform, calibrated_img.size(), cv::INTER_LINEAR);

    cv::Mat calibrated_img_col;
    cv::cvtColor(calibrated_img, calibrated_img_col, cv::COLOR_GRAY2BGR);

    for (auto * box : user_boxes_per_page[page-1]) {
      const std::vector<cv::Point2f> vec_box = {
        cv::Point2f{box->x, box->y},
        cv::Point2f{box->x + box->width, box->y},
        cv::Point2f{box->x + box->width, box->y + box->height},
        cv::Point2f{box->x, box->y + box->height}
      };
      std::vector<cv::Point> raster_box;
      raster_box.reserve(4);
      int min_x = INT_MAX;
      int min_y = INT_MAX;
      int max_x = INT_MIN;
      int max_y = INT_MIN;
      for (int i = 0; i < 4; ++i) {
        auto scaled = coord_scale(vec_box[i], src_img_size, dst_img_size);

        int x = round(scaled.x);
        int y = round(scaled.y);
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);

        raster_box.emplace_back(cv::Point(x, y));
      };

      // extract box content into file
      cv::Range rows(min_y, max_y);
      cv::Range cols(min_x, max_x);
      //printf("%d,%s: (%d,%d) -> (%d,%d)\n", copy, box->id.c_str(), min_x, min_y, max_x, max_y);
      cv::Mat subimg = calibrated_img(rows, cols);

      char * output_img_fname = nullptr;
      int nb = asprintf(&output_img_fname, "%s/subimg/raw-%d-%s.png", output_dir.c_str(), copy, box->id.c_str());
      (void) nb;
      printf("box fname: %s\n", output_img_fname);
      cv::imwrite(output_img_fname, subimg);
      free(output_img_fname);

      // draw polylines on the output image file
      cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(0, 0, 255), 2);
    }

    for (auto * box : corner_markers) {
      if (strncmp("marker barcode br", box->id.c_str(), 17) == 0)
        break;

      const std::vector<cv::Point2f> vec_box = {
        cv::Point2f{box->x, box->y},
        cv::Point2f{box->x + box->width, box->y},
        cv::Point2f{box->x + box->width, box->y + box->height},
        cv::Point2f{box->x, box->y + box->height}
      };
      std::vector<cv::Point> raster_box;
      raster_box.reserve(4);
      for (int i = 0; i < 4; ++i) {
        auto scaled = coord_scale(vec_box[i], src_img_size, dst_img_size);
        //printf("box %s corner %d: (%f, %f)\n", box->id.c_str(), i, scaled.x, scaled.y);
        raster_box.emplace_back(cv::Point(round(scaled.x), round(scaled.y)));
      };
      cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
    }

    std::filesystem::path input_img_path{argv[i]};
    std::filesystem::path output_img_path_fname = input_img_path.filename().replace_extension(".png");
    char * output_img_fname = nullptr;
    int nb = asprintf(&output_img_fname, "%s/cal-%s", output_dir.c_str(), output_img_path_fname.c_str());
    (void) nb;
    cv::imwrite(output_img_fname, calibrated_img_col);
    free(output_img_fname);

    /*cv::Mat with_markers;
    cv::cvtColor(img, with_markers, cv::COLOR_GRAY2BGR);
    std::string output_filename = std::string("/tmp/pout-") + std::to_string(i) + std::string(".png");
    cv::imwrite(output_filename, with_markers);*/
  }

  return 0;
}