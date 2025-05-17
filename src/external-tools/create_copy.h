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
    ARUCO,
    QR_EYE,
    CROSS,
    CUSTOM,
    NONE
};

std::string toString(MarkerType type);

/**
 * @brief Convertit une chaîne de caractères en type MarkerType
 *
 * Cette fonction prend une représentation textuelle d'un type de marqueur
 * et la convertit en son équivalent dans l'énumération MarkerType.
 *
 * @param typeStr Chaîne représentant le type de marqueur
 * @return MarkerType Type de marqueur correspondant, ou NONE si non reconnu
 */
MarkerType markerTypeFromString(const std::string& typeStr);

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

    /**
     * @brief Convertit le marqueur en chaîne de caractères
     *
     * @return std::string La représentation du marqueur sous forme de chaîne
     */
    std::string toString() const;

    /**
     * @brief Analyse une spécification de marqueur et crée l'objet Marker correspondant
     *
     * Cette fonction prend une chaîne de caractères qui décrit un marqueur
     * (par exemple "qrcode:encoded" ou "circle:outlined") et retourne
     * un objet Marker configuré selon cette spécification.
     *
     * @param spec Chaîne de spécification du marqueur au format "type[:encoded][:outlined]"
     * @return Marker Objet Marker configuré selon la spécification, ou marqueur vide si spec est vide ou "none"
     */
    static Marker parseMarker(const std::string& spec);
};

/**
 * @brief Structure pour configurer précisément les marqueurs sur une copie
 */
struct CopyMarkerConfig {
    Marker top_left;
    Marker top_right;
    Marker bottom_left;
    Marker bottom_right;
    Marker header;

    CopyMarkerConfig() {
    }

    CopyMarkerConfig(Marker tl, Marker tr, Marker bl, Marker br, Marker h)
        : top_left(tl), top_right(tr), bottom_left(bl), bottom_right(br), header(h) {
    }

    /**
     * @brief Convertit la configuration en chaîne de caractères
     *
     * @return std::string La représentation de la configuration au format "(marker1,marker2,marker3,marker4,marker5)"
     */
    std::string toString() const;

    /**
     * @brief Analyse une chaîne de caractères et configure l'objet en conséquence
     *
     * @param str Chaîne au format "(marker1,marker2,marker3,marker4,marker5)"
     * @param config Configuration à mettre à jour
     * @return int 0 en cas de succès, 1 en cas d'erreur
     */
    static int fromString(const std::string& str, CopyMarkerConfig& config);
};

inline std::ostream& operator<<(std::ostream& outs, const CopyMarkerConfig& config) {
    outs << ("(" + config.top_left.toString() + " | " + config.top_right.toString() + " | " +
             config.bottom_left.toString() + " | " + config.bottom_right.toString() + " | " + config.header.toString() +
             ")");
    return outs;
}

/**
 * @brief Structure contenant les paramètres de style pour la génération de copies
 */
struct CopyStyleParams {
    int encoded_marker_size = 15;   // Taille du marqueur encodé
    int unencoded_marker_size = 3;  // Taille du marqueur encodé
    int header_marker_size = 7;     // Taille du marqueur d'en-tête
    int stroke_width = 1;           // Épaisseur du trait des marqueurs
    int marker_margin = 3;          // Marge entre les marqueurs
    int grey_level = 0;             // 0 = noir et blanc, 1 = niveaux de gris, 2 = couleur
    int dpi = 300;                  // Résolution de l'image en DPI
    bool generating_content = true; // true = contenu généré, false = contenu statique
    int seed = 42;                  // Graine pour la génération aléatoire du contenu

    CopyStyleParams(int ems = 15, int ums = 3, int hms = 7, int sw = 1, int mm = 3, int gl = 0, int dpi = 300,
                    bool gc = true, int s = 42)
        : encoded_marker_size(ems), unencoded_marker_size(ums), header_marker_size(hms), stroke_width(sw),
          marker_margin(mm), grey_level(gl), dpi(dpi), generating_content(gc), seed(s) {
    }
};

/**
 * @brief Génère et exporte une copie paramétrée, enregistrée dans le répertoire ./copies
 *
 * @param style_params Paramètres de style pour la génération
 * @param marker_config Configuration des marqueurs à utiliser
 * @param filename Nom du fichier de sortie
 * @param verbose Si true, affiche tous les messages de sortie des commandes
 * @return true si la copie a été générée avec succès
 * @return false si une erreur est survenue
 */
bool create_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                 const std::string& filename = "copy", bool verbose = false);

#endif