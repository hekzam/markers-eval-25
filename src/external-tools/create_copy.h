#ifndef CREATE_COPY_H
#define CREATE_COPY_H

#include <string>
#include <vector>

enum MarkerConfig {
    QR_ALL_CORNERS = 1,        // QR codes avec données encodées dans tous les coins
    QR_BOTTOM_RIGHT_ONLY = 2,  // QR codes avec données encodées uniquement dans le coin bas-droit
    CIRCLES_WITH_QR_BR = 3,    // Cercles dans les trois premiers coins, QR code avec données dans le coin bas-droit
    TOP_CIRCLES_QR_BR = 4,     // Cercles en haut, rien en bas-gauche, QR code avec données en bas-droit
    CUSTOM_SVG_WITH_QR_BR = 5, // Marqueurs SVG personnalisés dans trois coins, QR code avec données en bas-droit
    ARUCO_WITH_QR_BR = 6,      // Différents marqueurs ArUco, QR code avec données en bas-droit
    TWO_ARUCO_WITH_QR_BR = 7,  // Deux marqueurs ArUco, rien en bas-gauche, QR code avec données en bas-droit
    CIRCLE_OUTLINES_WITH_QR_BR =
        8, // Cercles non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
    SQUARES_WITH_QR_BR =
        9, // Carrés dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
    SQUARE_OUTLINES_WITH_QR_BR =
        10 // Carrés non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
};

/**
 * @brief Constantes pour les types de marqueurs sous forme de chaînes
 */
const std::string QR_CODE = "qrcode";
const std::string DATAMATRIX = "datamatrix";
const std::string AZTEC = "aztec";
const std::string PDF417 = "pdf417-comp";
const std::string RMQR = "rmqr";
const std::string BARCODE = "barcode";
const std::string CIRCLE = "circle";
const std::string SQUARE = "square";
const std::string ARUCO_SVG = "aruco-svg";
const std::string CUSTOM_SVG = "custom-svg";

/**
 * @brief Structure représentant un marqueur avec son statut d'encodage
 */
struct Marker {
    std::string type; // Type du marqueur (sous forme de chaîne)
    bool encoded;     // Indique si le marqueur est encodé
    bool outlined;    // Indique si le marqueur est affiché uniquement avec son contour (non rempli)

    // Constructeur par défaut: pas de marqueur, non encodé, non contouré
    Marker() : type(""), encoded(false), outlined(false) {
    }

    // Constructeur avec paramètres
    Marker(const std::string& t, bool enc = false, bool out = false) : type(t), encoded(enc), outlined(out) {
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
            if (m.type.empty()) {
                return "none";
            }

            std::string result = m.type;
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
                return CopyMarkerConfig(Marker(QR_CODE),   // top_left
                                        Marker(QR_CODE),   // top_right
                                        Marker(QR_CODE),   // bottom_left
                                        Marker(QR_CODE),   // bottom_right
                                        Marker(RMQR, true) // header
                );

            case QR_BOTTOM_RIGHT_ONLY:
                return CopyMarkerConfig(Marker(),              // top_left
                                        Marker(),              // top_right
                                        Marker(),              // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case CIRCLES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(CIRCLE),        // top_left
                                        Marker(CIRCLE),        // top_right
                                        Marker(CIRCLE),        // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case TOP_CIRCLES_QR_BR:
                return CopyMarkerConfig(Marker(CIRCLE),        // top_left
                                        Marker(CIRCLE),        // top_right
                                        Marker(),              // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case CUSTOM_SVG_WITH_QR_BR:
                return CopyMarkerConfig(Marker(CUSTOM_SVG),    // top_left
                                        Marker(CUSTOM_SVG),    // top_right
                                        Marker(CUSTOM_SVG),    // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case ARUCO_WITH_QR_BR:
                return CopyMarkerConfig(Marker(ARUCO_SVG),     // top_left
                                        Marker(ARUCO_SVG),     // top_right
                                        Marker(ARUCO_SVG),     // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case TWO_ARUCO_WITH_QR_BR:
                return CopyMarkerConfig(Marker(ARUCO_SVG),     // top_left
                                        Marker(ARUCO_SVG),     // top_right
                                        Marker(),              // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case CIRCLE_OUTLINES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(CIRCLE, false, true), // top_left (outlined)
                                        Marker(CIRCLE, false, true), // top_right (outlined)
                                        Marker(CIRCLE, false, true), // bottom_left (outlined)
                                        Marker(QR_CODE, true),       // bottom_right
                                        Marker(QR_CODE, true)        // header
                );

            case SQUARES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(SQUARE),        // top_left
                                        Marker(SQUARE),        // top_right
                                        Marker(SQUARE),        // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
                );

            case SQUARE_OUTLINES_WITH_QR_BR:
                return CopyMarkerConfig(Marker(SQUARE, false, true), // top_left (outlined)
                                        Marker(SQUARE, false, true), // top_right (outlined)
                                        Marker(SQUARE, false, true), // bottom_left (outlined)
                                        Marker(QR_CODE, true),       // bottom_right
                                        Marker(QR_CODE, true)        // header
                );

            default:
                // Configuration par défaut: QR code encodé dans tous les coins
                return CopyMarkerConfig(Marker(QR_CODE, true), // top_left
                                        Marker(QR_CODE, true), // top_right
                                        Marker(QR_CODE, true), // bottom_left
                                        Marker(QR_CODE, true), // bottom_right
                                        Marker(QR_CODE, true)  // header
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
extern const std::vector<MarkerConfigInfo> marker_configs;

/**
 * @brief Structure contenant les paramètres de style pour la génération de copies
 */
struct CopyStyleParams {
    int encoded_marker_size;  // Taille du marqueur encodé
    int fiducial_marker_size; // Taille du marqueur de fiduciel
    int header_marker_size;   // Taille du marqueur d'en-tête
    int stroke_width;         // Largeur du trait
    int marker_margin;        // Marge du marqueur
    int grey_level;           // Niveau de gris

    // Constructeur avec valeurs par défaut
    CopyStyleParams(int ems = 15, int fms = 3, int hms = 7, int sw = 2, int mm = 3, int gl = 0)
        : encoded_marker_size(ems), fiducial_marker_size(fms), header_marker_size(hms), stroke_width(sw),
          marker_margin(mm), grey_level(gl) {
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
