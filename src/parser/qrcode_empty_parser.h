#ifndef ZXING_EMPTY_PARSER_H
#define ZXING_EMPTY_PARSER_H

/**
 * @file qrcode_empty_parser.h
 * @brief Module d'analyse pour des images avec peu ou pas de codes QR.
 *
 * Ce module est une variante du parser QR qui gère les cas où
 * l'image d'entrée contient peu ou pas de codes-barres, en
 * fournissant des valeurs par défaut.
 */

/**
 * @brief Analyse une image avec tolérance pour l'absence de codes QR
 * 
 * Cette fonction analyse l'image fournie pour:
 * 1. Détecter les codes QR/codes-barres présents
 * 2. Identifier les points de référence
 * 3. S'il y a moins de 4 codes-barres, retourne une matrice identité
 *    et des métadonnées par défaut plutôt qu'une erreur
 * 
 * @param img Image source à analyser
 * @param debug_img Image de débogage (disponible uniquement en mode DEBUG)
 * @param meta Structure de métadonnées à remplir (valeurs par défaut si non trouvées)
 * @param dst_corner_points Points de destination pour la transformation
 * @param flag_barcode Type de code-barres à détecter
 * 
 * @return Une matrice de transformation optionnelle, matrice identité en cas d'échec partiel
 */
std::optional<cv::Mat> qrcode_empty_parser(const cv::Mat& img,
#ifdef DEBUG
                                           cv::Mat debug_img,
#endif
                                           Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                           int flag_barcode);
#endif // ZXING_EMPTY_PARSER_H