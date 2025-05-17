#ifndef CIRCLE_PARSER_H
#define CIRCLE_PARSER_H

/**
 * @file circle_parser.h
 * @brief Module d'analyse et de détection de cercles dans une image.
 *
 * Ce module permet d'identifier les cercles présents dans une image
 * en utilisant la transformée de Hough et de localiser des points
 * de référence à partir de ces cercles.
 */

/**
 * @brief Analyse une image pour détecter des cercles et un code-barres
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les codes-barres présents et identifier le code de coin
 * 2. Utiliser la transformée de Hough pour détecter les cercles
 * 3. Utiliser les centres des cercles détectés comme points de référence
 * 4. Calculer une matrice de transformation basée sur les points détectés
 * 
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir depuis le code-barres
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter
 * 
 * @return Une matrice de transformation optionnelle, ou vide en cas d'échec
 */
std::optional<cv::Mat> circle_parser(const cv::Mat& img,
#ifdef DEBUG
                                     cv::Mat debug_img,
#endif
                                     Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

#endif