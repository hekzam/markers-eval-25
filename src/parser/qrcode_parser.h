#ifndef QRCODE_PARSER_H
#define QRCODE_PARSER_H

/**
 * @file qrcode_parser.h
 * @brief Module d'analyse et de détection de codes QR dans une image.
 *
 * Ce module permet d'identifier les codes QR présents dans une image
 * et d'extraire les points de référence des coins spécifiques identifiés
 * par des préfixes particuliers ("hz").
 */

/**
 * @brief Analyse une image pour détecter des codes QR et extraire des informations
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les codes QR/codes-barres présents
 * 2. Identifier les codes QR de coin (préfixés par "hz")
 * 3. Extraire les métadonnées des codes détectés
 * 4. Calculer une matrice de transformation basée sur les points détectés
 * 
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter
 * 
 * @return Une matrice de transformation optionnelle, ou vide en cas d'échec
 */
std::optional<cv::Mat> qrcode_parser(const cv::Mat& img,
#ifdef DEBUG
                                     cv::Mat debug_img,
#endif
                                     Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);
#endif // QRCODE_PARSER_H