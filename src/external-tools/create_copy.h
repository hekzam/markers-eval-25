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

/**
 * @brief Énumération des différents types de marqueurs supportés
 * 
 * Cette énumération définit tous les types de marqueurs que le système
 * peut générer et utiliser pour le repérage dans les documents.
 */
enum class MarkerType {
    QR_CODE,       // Code QR standard
    MICRO_QR_CODE, // Version réduite du code QR
    DATAMATRIX,    // Code DataMatrix
    AZTEC,         // Code Aztec
    PDF417,        // Code-barres 2D PDF417
    RMQR,          // Code QR rectangulaire (Rectangular Micro QR Code)
    BARCODE,       // Code-barres linéaire (Code 128 par défaut)
    CIRCLE,        // Marqueur circulaire simple
    SQUARE,        // Marqueur carré simple
    ARUCO,         // Marqueur ArUco pour la réalité augmentée
    QR_EYE,        // Marqueur en forme d'œil de code QR
    CROSS,         // Marqueur en forme de croix
    CUSTOM,        // Marqueur personnalisé
    NONE           // Aucun marqueur
};

/**
 * @brief Convertit un type de marqueur en sa représentation textuelle
 * 
 * @param type Le type de marqueur à convertir
 * @return std::string La chaîne correspondant au type de marqueur
 */
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
     * (par exemple "qrcode:enc" ou "circle:out") et retourne
     * un objet Marker configuré selon cette spécification.
     *
     * @param spec Chaîne de spécification du marqueur au format "type[:enc][:out]"
     * @return Marker Objet Marker configuré selon la spécification, ou marqueur vide si spec est vide ou "#"
     */
    static Marker parseMarker(const std::string& spec);
};

/**
 * @brief Structure pour configurer précisément les marqueurs sur une copie
 * 
 * Cette structure permet de définir le type et les propriétés des marqueurs
 * pour chacun des quatre coins du document ainsi qu'un marqueur d'en-tête optionnel.
 */
struct CopyMarkerConfig {
    Marker top_left;     // Marqueur en haut à gauche
    Marker top_right;    // Marqueur en haut à droite
    Marker bottom_left;  // Marqueur en bas à gauche
    Marker bottom_right; // Marqueur en bas à droite
    Marker header;       // Marqueur d'en-tête (généralement au centre)

    // Constructeur par défaut
    CopyMarkerConfig() {
    }

    /**
     * @brief Constructeur avec spécification complète des marqueurs
     * 
     * @param tl Marqueur en haut à gauche
     * @param tr Marqueur en haut à droite
     * @param bl Marqueur en bas à gauche
     * @param br Marqueur en bas à droite
     * @param h Marqueur d'en-tête
     */
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

/**
 * @brief Surcharge de l'opérateur d'insertion pour faciliter l'affichage de la configuration
 * 
 * @param outs Flux de sortie
 * @param config Configuration à afficher
 * @return std::ostream& Référence au flux de sortie
 */
inline std::ostream& operator<<(std::ostream& outs, const CopyMarkerConfig& config) {
    std::vector<Marker> markers = {config.top_left, config.top_right, config.bottom_left, config.bottom_right, config.header};
    std::string result = "";
    
    int count = 1;
    Marker currentMarker = markers[0];
    
    for (size_t i = 1; i < markers.size(); ++i) {
        if (markers[i].toString() == currentMarker.toString()) {
            count++;
        } else {
            if (count > 1) {
                result += std::to_string(count) + "*" + (currentMarker.toString()) + " | ";
            } else {
                result += (currentMarker.toString()) + " | ";
            }
            currentMarker = markers[i];
            count = 1;
        }
    }
    
    if (count > 1) {
        result += std::to_string(count) + "*" + (currentMarker.toString());
    } else {
        result += (currentMarker.toString());
    }
    
    outs << result;
    return outs;
}

/**
 * @brief Structure contenant les paramètres de style pour la génération de copies
 */
struct CopyStyleParams {
    int encoded_marker_size = 15;   // Taille en mm des marqueurs encodés (QR, DataMatrix, etc.)
    int unencoded_marker_size = 3;  // Taille en mm des marqueurs non encodés (cercles, carrés, etc.)
    int header_marker_size = 7;     // Taille en mm du marqueur d'en-tête
    int stroke_width = 1;           // Épaisseur du trait des marqueurs (en points)
    int marker_margin = 3;          // Marge en mm entre les marqueurs et le bord du document
    int grey_level = 0;             // Niveau de gris: 0 = noir et blanc, 1 = niveaux de gris, 2 = couleur
    int dpi = 300;                  // Résolution de l'image en points par pouce (DPI)
    bool generating_content = true; // Si true, génère du contenu aléatoire, sinon ne génère pas de contenu
    int seed = 42;                  // Graine pour la génération aléatoire du contenu
    int content_margin_x = 5;      // Marge horizontale en mm pour le contenu par rapport aux bords
    int content_margin_y = 5;      // Marge verticale en mm pour le contenu par rapport aux bords

    /**
     * @brief Constructeur avec paramètres optionnels
     * 
     * @param ems Taille des marqueurs encodés (encoded_marker_size)
     * @param ums Taille des marqueurs non encodés (unencoded_marker_size)
     * @param hms Taille du marqueur d'en-tête (header_marker_size)
     * @param sw Épaisseur du trait (stroke_width)
     * @param mm Marge des marqueurs (marker_margin)
     * @param gl Niveau de gris (grey_level)
     * @param dpi Résolution en DPI
     * @param gc Génération de contenu (generating_content)
     * @param s Graine aléatoire (seed)
     * @param cmx Marge horizontale du contenu (content_margin_x)
     * @param cmy Marge verticale du contenu (content_margin_y)
     */
    CopyStyleParams(int ems = 15, int ums = 3, int hms = 7, int sw = 1, int mm = 3, int gl = 0, int dpi = 300,
                    bool gc = true, int s = 42, int cmx = 5, int cmy = 5)
        : encoded_marker_size(ems), unencoded_marker_size(ums), header_marker_size(hms), stroke_width(sw),
          marker_margin(mm), grey_level(gl), dpi(dpi), generating_content(gc), seed(s), 
          content_margin_x(cmx), content_margin_y(cmy) {
    }
};

/**
 * @brief Génère et exporte une copie paramétrée, enregistrée dans le répertoire ./copies
 *
 * Cette fonction utilise Typst pour générer un document avec les marqueurs spécifiés.
 * Le document est exporté en PNG avec les métadonnées associées (position des marqueurs)
 * stockées dans un fichier JSON séparé.
 *
 * @param style_params Paramètres de style pour la génération
 * @param marker_config Configuration des marqueurs à utiliser
 * @param filename Nom du fichier de sortie (sans extension)
 * @param verbose Si true, affiche tous les messages de sortie des commandes
 * @return true si la copie a été générée avec succès
 * @return false si une erreur est survenue
 */
bool create_copy(const CopyStyleParams& style_params, const CopyMarkerConfig& marker_config,
                 const std::string& filename = "copy", bool verbose = false);

#endif