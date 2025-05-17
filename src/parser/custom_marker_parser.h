#ifndef CUSTOM_PARSER_H
#define CUSTOM_PARSER_H

/**
 * @file custom_marker_parser.h
 * @brief Module d'analyse et de détection de marqueurs personnalisés.
 *
 * Ce module permet d'identifier des marqueurs personnalisés dans une image
 * en utilisant des techniques de seuillage et d'analyse de contours.
 */

/**
 * @brief Analyse une image pour détecter des marqueurs personnalisés
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Convertir l'image en niveaux de gris
 * 2. Appliquer un seuillage binaire
 * 3. Détecter les contours et filtrer ceux qui correspondent à des marqueurs
 * 
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter (non utilisé)
 * 
 * @return Une matrice de transformation optionnelle, actuellement toujours vide
 */
std::optional<cv::Mat> custom_marker_parser(const cv::Mat& img,
#ifdef DEBUG
                                            cv::Mat debug_img,
#endif
                                            Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                            int flag_barcode);

#endif