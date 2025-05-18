#ifndef DEFAULT_PARSER_H
#define DEFAULT_PARSER_H

/**
 * @file default_parser.h
 * @brief Module d'analyse par défaut (ne fait rien).
 *
 * Ce module fournit une implémentation par défaut du parser
 * qui ne fait aucun traitement et renvoie systématiquement
 * un résultat vide.
 */

/**
 * @brief Parser par défaut qui ne réalise aucune analyse
 * 
 * Cette fonction implémente l'interface standard des parsers
 * mais ne réalise aucun traitement. Elle renvoie toujours
 * un résultat vide (std::optional sans valeur).
 * 
 * @param img Image source à analyser (non utilisée)
 * @param debug_img Image de débogage (non utilisée)
 * @param meta Structure de métadonnées (non modifiée)
 * @param dst_corner_points Points de destination pour la transformation (non modifiés)
 * @param flag_barcode Type de code-barres à détecter (non utilisé)
 * 
 * @return Toujours un std::optional vide
 */
std::optional<cv::Mat> default_parser(const cv::Mat& img,
#ifdef DEBUG
                                      cv::Mat debug_img,
#endif
                                      Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

#endif