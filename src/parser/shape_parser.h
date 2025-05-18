#ifndef SHAPE_PARSER_H
#define SHAPE_PARSER_H

/**
 * @file shape_parser.h
 * @brief Module d'analyse et de détection de formes dans une image.
 *
 * Ce module permet d'identifier les formes présentes dans une image
 * et de localiser des points de référence à partir de ces formes.
 */

/**
 * @brief Analyse une image pour détecter des formes et un code-barres
 *
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les codes-barres présents
 * 2. Identifier les formes et extraire leurs centres
 * 3. Calculer une matrice de transformation basée sur les points détectés
 *
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter
 *
 * @return Une matrice de transformation optionnelle, ou vide en cas d'échec
 */
std::optional<cv::Mat> shape_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

#endif