#ifndef CENTER_MARKER_PARSER_H
#define CENTER_MARKER_PARSER_H

/**
 * @file center_marker_parser.h
 * @brief Module d'analyse de marqueurs centraux dans une image.
 *
 * Ce module permet d'identifier des marqueurs centraux dans une image
 * et de localiser les points de référence les plus proches des coins
 * pour établir une transformation.
 */

/**
 * @brief Analyse une image pour détecter des marqueurs et un code-barres central
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les codes-barres présents
 * 2. Identifier spécifiquement un code QR d'en-tête (préfixé par "hztc")
 * 3. Détecter des formes dans l'image et extraire leurs centres
 * 4. Déterminer quels points sont les plus proches des coins de l'image
 * 5. Calculer une matrice de transformation basée sur ces points
 * 
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir depuis le code-barres d'en-tête
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter
 * 
 * @return Une matrice de transformation optionnelle, ou vide en cas d'échec
 */
std::optional<cv::Mat> center_marker_parser(const cv::Mat& img,
#ifdef DEBUG
                                            cv::Mat debug_img,
#endif
                                            Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                            int flag_barcode);

#endif