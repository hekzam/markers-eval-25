#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#include "external-tools/create_copy.h"

/**
 * @file parser_helper.h
 * @brief Module d'analyse des images pour la détection de marqueurs et de codes-barres.
 *
 * Ce module fournit des fonctions pour détecter et analyser différents types de marqueurs
 * (QR codes, cercles, ArUco, etc.) dans une image donnée. Il inclut également des utilitaires
 * pour le traitement des images et la gestion des transformations géométriques.
 */

#ifdef ENABLE_ZBAR
#include <zbar.h>
#else
#include <ZXing/ReadBarcode.h>
#endif
#include <common.h>

/**
 * @brief Convertit un type de parseur (ParserType) en chaîne de caractères
 *
 * @param parser_type Type de parseur à convertir
 * @return std::string Chaîne de caractères représentant le type de parseur
 */
std::string parser_type_to_string(ParserType parser_type);

/**
 * @brief Convertit une chaîne de caractères en type de parseur (ParserType)
 *
 * @param parser_type_str Chaîne de caractères représentant le type de parseur
 * @return ParserType Le type de parseur correspondant, ou ParserType::QRCODE par défaut
 */
ParserType string_to_parser_type(const std::string& parser_type_str);

/**
 * @brief Structure représentant un code-barres détecté dans l'image
 */
struct DetectedBarcode {
    std::string content;                   ///< Contenu décodé du code-barres
    std::vector<cv::Point2f> bounding_box; ///< Points délimitant le contour du code-barres
};

/**
 * @brief Identifie tous les codes-barres présents dans une image
 *
 * @param img Image en niveau de gris (CV_8U) à analyser
 * @param flags Type de code-barres à rechercher (dépend de la bibliothèque utilisée)
 * @return std::vector<DetectedBarcode> Liste des codes-barres détectés avec leur contenu et position
 * @throw std::invalid_argument Si l'image n'est pas au format CV_8U
 */
std::vector<DetectedBarcode> identify_barcodes(const cv::Mat& img,
#ifdef ENABLE_ZBAR
                                               zbar_symbol_type_t flags = zbar::ZBAR_QRCODE
#else
                                               ZXing::BarcodeFormats flags = ZXing::BarcodeFormat::QRCode
#endif
);

/**
 * @brief Calcule la transformation affine à partir des points de coin trouvés et attendus
 *
 * @param found_corner_mask Masque binaire indiquant quels coins ont été trouvés (bit 0 pour coin haut gauche, etc.)
 * @param expected_corner_points Coordonnées attendues des coins dans l'image de destination
 * @param found_corner_points Coordonnées des coins trouvés dans l'image source
 * @return std::optional<cv::Mat> Matrice de transformation affine ou nullopt si moins de 3 coins ont été trouvés
 */
std::optional<cv::Mat> get_affine_transform(int found_corner_mask,
                                            const std::vector<cv::Point2f>& expected_corner_points,
                                            const std::vector<cv::Point2f>& found_corner_points);

/**
 * @brief Différencie les AtomicBox en les classant par type et page.
 *
 * La fonction sépare les boîtes en trois catégories :
 * - Les marqueurs généraux (markers) dont l'identifiant commence par "marker barcode ".
 * - Les marqueurs de coin (corner_markers) pour lesquels l'identifiant contient les suffixes tl, tr, bl, br, tc.
 * - Les boîtes utilisateur (user_boxes_per_page), regroupées par numéro de page.
 *
 * Un contrôle est effectué pour s'assurer que tous les marqueurs de coin sont présents. En cas d'absence,
 * une exception std::invalid_argument est levée.
 *
 * @param boxes Vecteur de pointeurs partagés sur les AtomicBox à différencier.
 * @param corner_markers Vecteur de pointeurs partagés (de taille 5) où chaque index correspond à un coin :
 *        TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, TOP_CENTER.
 * @param user_boxes_per_page Vecteur de vecteurs de pointeurs partagés qui regroupe les boîtes utilisateur par page.
 *
 * @throw std::invalid_argument Si un ou plusieurs marqueurs de coin sont manquants dans la description JSON.
 */
void differentiate_atomic_boxes(std::vector<std::shared_ptr<AtomicBox>>& boxes,
                                std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page);

/**
 * @brief Calcule le centre des marqueurs de coin
 *
 * Pour chaque marqueur de coin fourni, la fonction calcule le centre de sa boîte englobante
 * puis applique une transformation de redimensionnement pour adapter ce centre aux dimensions
 * de l'image destination
 *
 * @param corner_markers Vecteur de pointeurs partagés sur les marqueurs de coin dans l'image source
 * @param src_img_size Taille de l'image source (largeur, hauteur)
 * @param dst_img_size Taille de l'image destination (largeur, hauteur)
 * @return std::vector<cv::Point2f> Vecteur contenant les points centraux des marqueurs redimensionnés
 */
std::vector<cv::Point2f>
calculate_center_of_marker(const std::vector<std::optional<std::shared_ptr<AtomicBox>>>& corner_markers,
                           const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);

/**
 * @brief Redresse une image en appliquant une transformation affine.
 *
 * Cette fonction applique une transformation affine à une image pour la redresser.
 * Ensuite, l'image redressée est convertie en couleur BGR.
 *
 * @param img Image en niveaux de gris à redresser
 * @param affine_transform Matrice de transformation affine à appliquer
 * @return cv::Mat Image redressée et convertie en couleur BGR
 */
cv::Mat redress_image(cv::Mat img, cv::Mat affine_transform);

/**
 * @brief Exécute le parseur spécifié sur l'image donnée
 *
 * @param parser_type Type de parseur à utiliser
 * @param img Image à analyser
 * @param meta Métadonnées à remplir pendant l'analyse
 * @param dst_corner_points Points de coin de destination pour la transformation
 * @param flag_barcode Type de code-barres à rechercher (par défaut : QRCode)
 * @return std::optional<cv::Mat> Matrice de transformation affine si trouvée, sinon nullopt
 */
std::optional<cv::Mat> run_parser(const ParserType& parser_type, cv::Mat img,
#ifdef DEBUG
                                  cv::Mat debug_img,
#endif
                                  Metadata& meta, std::vector<cv::Point2f>& dst_corner_points,
                                  int flag_barcode = (int) ZXing::BarcodeFormat::QRCode);

int copy_config_to_flag(const CopyMarkerConfig& copy_marker_config);

/**
 * @brief Sélectionne le code-barres correspondant au coin inférieur droit
 *
 * @param barcodes Liste des codes-barres détectés dans l'image
 * @return std::optional<DetectedBarcode> Code-barres du coin inférieur droit ou nullopt si non trouvé
 */
std::optional<DetectedBarcode> select_bottom_right_corner(const std::vector<DetectedBarcode>& barcodes);

template <typename T>
std::vector<T> smaller_parse(const cv::Mat& img,
#ifdef DEBUG
                             cv::Mat debug_img,
#endif
                             std::vector<T> (*parse_func)(const cv::Mat&, const cv::Point2i&), float size = 0.2) {
    std::vector<T> parsed_data;

    // top-left corner
    cv::Mat img_tl = img(cv::Rect(0, 0, img.cols * size, img.rows * size));
#ifdef DEBUG
    cv::polylines(debug_img,
                  std::vector<cv::Point2i>{ { 0, 0 },
                                            { (int) (img.cols * size), 0 },
                                            { (int) (img.cols * size), (int) (img.rows * size) },
                                            { 0, (int) (img.rows * size) } },
                  true, cv::Scalar(255, 0, 0), 2);
#endif
    auto parsed_tl = parse_func(img_tl, { 0, 0 });
    parsed_data.insert(parsed_data.end(), parsed_tl.begin(), parsed_tl.end());

    // top-right corner
    cv::Mat img_tr = img(cv::Rect(img.cols * (1 - size), 0, img.cols * size, img.rows * size));
#ifdef DEBUG
    cv::polylines(debug_img,
                  std::vector<cv::Point2i>{ { (int) (img.cols * (1 - size)), 0 },
                                            { img.cols, 0 },
                                            { img.cols, (int) (img.rows * size) },
                                            { (int) (img.cols * (1 - size)), (int) (img.rows * size) } },
                  true, cv::Scalar(255, 0, 0), 2);
#endif
    auto parsed_tr = parse_func(img_tr, { (int) (img.cols * (1 - size)), 0 });
    parsed_data.insert(parsed_data.end(), parsed_tr.begin(), parsed_tr.end());

    // bottom-left corner
    cv::Mat img_bl = img(cv::Rect(0, img.rows * (1 - size), img.cols * size, img.rows * size));
#ifdef DEBUG
    cv::polylines(debug_img,
                  std::vector<cv::Point2i>{ { 0, (int) (img.rows * (1 - size)) },
                                            { (int) (img.cols * size), (int) (img.rows * (1 - size)) },
                                            { (int) (img.cols * size), (int) (img.rows) },
                                            { 0, img.rows } },
                  true, cv::Scalar(255, 0, 0), 2);
#endif
    auto parsed_bl = parse_func(img_bl, { 0, (int) (img.rows * (1 - size)) });
    parsed_data.insert(parsed_data.end(), parsed_bl.begin(), parsed_bl.end());

    // bottom-right corner
    cv::Mat img_br = img(cv::Rect(img.cols * (1 - size), img.rows * (1 - size), img.cols * size, img.rows * size));
#ifdef DEBUG
    cv::polylines(debug_img,
                  std::vector<cv::Point2i>{ { (int) (img.cols * (1 - size)), (int) (img.rows * (1 - size)) },
                                            { img.cols, (int) (img.rows * (1 - size)) },
                                            { img.cols, img.rows },
                                            { (int) (img.cols * (1 - size)), img.rows } },
                  true, cv::Scalar(255, 0, 0), 2);
#endif
    auto parsed_br = parse_func(img_br, { (int) (img.cols * (1 - size)), (int) (img.rows * (1 - size)) });
    parsed_data.insert(parsed_data.end(), parsed_br.begin(), parsed_br.end());

    return parsed_data;
}

#endif