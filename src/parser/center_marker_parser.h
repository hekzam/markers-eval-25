#ifndef CENTER_MARKER_PARSER_H
#define CENTER_MARKER_PARSER_H

std::optional<cv::Mat> center_marker_parser(const cv::Mat& img,
#ifdef DEBUG
                                            cv::Mat debug_img,
#endif
                                            Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                            int flag_barcode);

#endif