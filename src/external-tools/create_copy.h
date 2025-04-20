#ifndef CREATE_COPY_H
#define CREATE_COPY_H

/**
 * @file create_copy.h
 * @brief Module de génération de copies avec différents types de marqueurs.
 *
 * Ce fichier définit les structures et fonctions nécessaires pour créer des copies
 * de documents avec divers types de marqueurs (QR codes, cercles, carrés, ArUco, etc.).
 * Il propose une interface flexible pour configurer l'apparence et le placement des marqueurs,
 * ainsi que différentes configurations prédéfinies pour s'adapter à diverses utilisations.
 */

#include <string>
#include <vector>

#define QR_ALL_CORNERS 1       // QR codes avec données encodées dans tous les coins
#define QR_BOTTOM_RIGHT_ONLY 2 // QR codes avec données encodées uniquement dans le coin bas-droit
#define CIRCLES_WITH_QR_BR 3   // Cercles dans les trois premiers coins, QR code avec données dans le coin bas-droit
#define TOP_CIRCLES_QR_BR 4    // Cercles en haut, rien en bas-gauche, QR code avec données en bas-droit
#define CUSTOM_WITH_QR_BR 5    // Marqueurs SVG personnalisés dans trois coins, QR code avec données en bas-droit
#define ARUCO_WITH_QR_BR 6     // Différents marqueurs ArUco, QR code avec données en bas-droit
#define TWO_ARUCO_WITH_QR_BR 7 // Deux marqueurs ArUco, rien en bas-gauche, QR code avec données en bas-droit
#define CIRCLE_OUTLINES_WITH_QR_BR \
    8 // Cercles non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
#define SQUARES_WITH_QR_BR \
    9 // Carrés dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
#define SQUARE_OUTLINES_WITH_QR_BR \
    10 // Carrés non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit

// Remplacer les macros de types de marqueurs par un enum class
enum class MarkerType {
    QR_CODE,
    MICRO_QR_CODE,
    DATAMATRIX,
    AZTEC,
    PDF417,
    RMQR,
    BARCODE,
    CIRCLE,
    SQUARE,
    TRIANGLE,
    ARUCO,
    QR_EYE,
    CUSTOM,
    NONE // Pour représenter l'absence de marqueur
};

// Fonction auxiliaire pour convertir les types d'énumération en chaînes
inline std::string toString(MarkerType type) {
    switch (type) {
        case MarkerType::QR_CODE:
            return "qrcode";
        case MarkerType::MICRO_QR_CODE:
            return "micro-qr";
        case MarkerType::DATAMATRIX:
            return "datamatrix";
        case MarkerType::AZTEC:
            return "aztec";
        case MarkerType::PDF417:
            return "pdf417";
        case MarkerType::RMQR:
            return "rmqr";
        case MarkerType::BARCODE:
            return "code128";
        case MarkerType::CIRCLE:
            return "circle";
        case MarkerType::SQUARE:
            return "square";
        case MarkerType::TRIANGLE:
            return "triangle";
        case MarkerType::ARUCO:
            return "aruco";
        case MarkerType::QR_EYE:
            return "qr-eye";
        case MarkerType::CUSTOM:
            return "custom";
        case MarkerType::NONE:
            return "";
        default:
            return "";
    }
}

/**
 * @brief Structure représentant un marqueur avec son statut d'encodage
 */
struct Marker {
    MarkerType type; // Type du marqueur (enum)
    bool encoded;    // Indique si le marqueur est encodé
    bool outlined;   // Indique si le marqueur est affiché uniquement avec son contour (non rempli)

    // Constructeur par défaut: pas de marqueur, non encodé, non contouré
    Marker() : type(MarkerType::NONE), encoded(false), outlined(false) {
    }

    // Constructeur avec paramètres
    Marker(MarkerType t, bool enc = false, bool out = false) : type(t), encoded(enc), outlined(out) {
    }
};

/**
 * @brief Structure pour configurer précisément les marqueurs sur une copie
 */
struct CopyMarkerConfig {
    Marker top_left;     // Marqueur coin supérieur gauche
    Marker top_right;    // Marqueur coin supérieur droit
    Marker bottom_left;  // Marqueur coin inférieur gauche
    Marker bottom_right; // Marqueur coin inférieur droit
    Marker header;       // Marqueur d'en-tête

    // Constructeur par défaut: aucun marqueur
    CopyMarkerConfig() {
    }

    // Constructeur avec marqueurs
    CopyMarkerConfig(Marker tl, Marker tr, Marker bl, Marker br, Marker h)
        : top_left(tl), top_right(tr), bottom_left(bl), bottom_right(br), header(h) {
    }

    // Méthode pour convertir la configuration en chaîne de caractères
    std::string toString() const {
        auto formatMarker = [](const Marker& m) -> std::string {
            if (m.type == MarkerType::NONE) {
                return "none";
            }

            std::string result = ::toString(m.type);
            if (m.outlined) {
                result += "-outlined";
            }
            if (m.encoded) {
                result += "-encoded";
            }
            return result;
        };

        return "(" + formatMarker(top_left) + "," + formatMarker(top_right) + "," + formatMarker(bottom_left) + "," +
               formatMarker(bottom_right) + "," + formatMarker(header) + ")";
    }

    // Configurations prédéfinies selon les valeurs de MarkerConfig
    static CopyMarkerConfig getConfigById(int configId) {
        switch (configId) {
            case QR_ALL_CORNERS:
                return CopyMarkerConfig(Marker(MarkerType::QR_CODE),   // top_left
                                        Marker(MarkerType::QR_CODE),   // top_right
                                        Marker(MarkerType::QR_CODE),   // bottom_left
                                        Marker(MarkerType::QR_CODE),   // bottom_right
                                        Marker(MarkerType::RMQR, true) // header
                );

            case QR_BOTTOM_RIGHT_ONLY:
                return CopyMarkerConfig(Marker(),                          // top_left
                                        Marker(),                          // top_right
                                        Marker(),                          // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case CIRCLES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::CIRCLE),        // top_left
                                        Marker(MarkerType::CIRCLE),        // top_right
                                        Marker(MarkerType::CIRCLE),        // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case TOP_CIRCLES_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::CIRCLE),        // top_left
                                        Marker(MarkerType::CIRCLE),        // top_right
                                        Marker(),                          // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case CUSTOM_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::CUSTOM),        // top_left
                                        Marker(MarkerType::CUSTOM),        // top_right
                                        Marker(MarkerType::CUSTOM),        // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case ARUCO_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::ARUCO),         // top_left
                                        Marker(MarkerType::ARUCO),         // top_right
                                        Marker(MarkerType::ARUCO),         // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case TWO_ARUCO_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::ARUCO),         // top_left
                                        Marker(MarkerType::ARUCO),         // top_right
                                        Marker(),                          // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case CIRCLE_OUTLINES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::CIRCLE, false, true), // top_left (outlined)
                                        Marker(MarkerType::CIRCLE, false, true), // top_right (outlined)
                                        Marker(MarkerType::CIRCLE, false, true), // bottom_left (outlined)
                                        Marker(MarkerType::QR_CODE, true),       // bottom_right
                                        Marker(MarkerType::QR_CODE, true)        // header
                );

            case SQUARES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::SQUARE),        // top_left
                                        Marker(MarkerType::SQUARE),        // top_right
                                        Marker(MarkerType::SQUARE),        // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );

            case SQUARE_OUTLINES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(MarkerType::SQUARE, false, true), // top_left (outlined)
                                        Marker(MarkerType::SQUARE, false, true), // top_right (outlined)
                                        Marker(MarkerType::SQUARE, false, true), // bottom_left (outlined)
                                        Marker(MarkerType::QR_CODE, true),       // bottom_right
                                        Marker(MarkerType::QR_CODE, true)        // header
                );

            default:
                // Configuration par défaut: QR code encodé dans tous les coins
                return CopyMarkerConfig(Marker(MarkerType::QR_CODE, true), // top_left
                                        Marker(MarkerType::QR_CODE, true), // top_right
                                        Marker(MarkerType::QR_CODE, true), // bottom_left
                                        Marker(MarkerType::QR_CODE, true), // bottom_right
                                        Marker(MarkerType::QR_CODE, true)  // header
                );
        }
    }
};

/**
 * @brief Structure contenant les informations sur les configurations de marqueurs
 */
struct MarkerConfigInfo {
    int id;
    std::string description;
};

/**
 * @brief Liste des configurations de marqueurs disponibles
 */
const std::vector<MarkerConfigInfo> marker_configs = {
    { QR_ALL_CORNERS, "QR codes in all corners" },
    { QR_BOTTOM_RIGHT_ONLY, "QR code only in bottom-right corner" },
    { CIRCLES_WITH_QR_BR, "Circles in first three corners, QR code in bottom-right" },
    { TOP_CIRCLES_QR_BR, "Circles on top, nothing in bottom-left, QR code in bottom-right" },
    { CUSTOM_WITH_QR_BR, "Custom SVG markers in three corners, QR code in bottom-right" },
    { ARUCO_WITH_QR_BR, "ArUco markers, QR code in bottom-right" },
    { TWO_ARUCO_WITH_QR_BR, "Two ArUco markers, nothing in bottom-left, QR code in bottom-right" },
    { CIRCLE_OUTLINES_WITH_QR_BR, "Circle outlines in first three corners, QR code in bottom-right" },
    { SQUARES_WITH_QR_BR, "Squares in first three corners, QR code in bottom-right" },
    { SQUARE_OUTLINES_WITH_QR_BR, "Square outlines in first three corners, QR code in bottom-right" }
};

/**
 * @brief Structure contenant les paramètres de style pour la génération de copies
 */
struct CopyStyleParams {
    int encoded_marker_size;
    int unencoded_marker_size;
    int header_marker_size;
    int stroke_width;
    int marker_margin;
    int grey_level;
    int dpi;

    // Constructeur avec valeurs par défaut
    CopyStyleParams(int ems = 15, int ums = 3, int hms = 7, int sw = 2, int mm = 3, int gl = 0, int dpi = 300)
        : encoded_marker_size(ems), unencoded_marker_size(ums), header_marker_size(hms), stroke_width(sw),
          marker_margin(mm), grey_level(gl), dpi(dpi) {
    }
};

/**
 * @brief Génère et exporte une copie paramétrée, enregistrée dans le répertoire ./copies
 *
 * @param style_params Paramètres de style pour la génération
 * @param marker_config Configuration des marqueurs à utiliser
 * @param filename Nom du fichier de sortie
 * @return true si la copie a été générée avec succès
 * @return false si une erreur est survenue
 */
bool create_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                 const std::string& filename = "copy");

#endif