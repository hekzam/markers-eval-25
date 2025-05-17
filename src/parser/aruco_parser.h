#ifndef ARUCO_PARSER_H
#define ARUCO_PARSER_H

/**
 * @file aruco_parser.h
 * @brief Module d'analyse et de détection de marqueurs ArUco dans une image.
 *
 * Ce module permet d'identifier des marqueurs ArUco présents dans une image
 * et de les utiliser comme points de référence pour établir une transformation.
 * Il utilise spécifiquement les marqueurs ArUco du dictionnaire DICT_4X4_1000.
 */

/**
 * @brief Analyse une image pour détecter des marqueurs ArUco et un code-barres
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les marqueurs ArUco avec des IDs spécifiques (190, 997, 999)
 * 2. Détecter un code-barres dans le coin inférieur droit
 * 3. Utiliser les centres des marqueurs détectés comme points de référence
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
std::optional<cv::Mat> aruco_parser(const cv::Mat& img,
#ifdef DEBUG
                                    cv::Mat debug_img,
#endif
                                    Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

#endif