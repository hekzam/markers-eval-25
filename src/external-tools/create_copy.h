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
 * @brief Génère et exporte une copie paramétrée, enregistrée dans le répertoire ./copies
 * 
 * @param encoded_marker_size Taille des marqueurs encodés
 * @param fiducial_marker_size Taille des marqueurs fiduciaires
 * @param stroke_width Largeur du trait des marqueurs
 * @param marker_margin Marge autour des marqueurs
 * @param nb_copies Nombre de copies à générer
 * @param duplex_printing Mode d'impression recto-verso (0: simple face, 1: recto-verso)
 * @param marker_config Configuration des marqueurs
 * @param grey_level Niveau de gris
 * @param header_marker Affiche un marqueur d'entête
 * @param filename Nom du fichier de sortie
 * @return true si la copie a été générée avec succès
 * @return false si une erreur est survenue
 */
bool create_copy(int encoded_marker_size, int fiducial_marker_size, int stroke_width, int marker_margin, int nb_copies,
                 int duplex_printing, int marker_config, int grey_level, int header_marker,
                 const std::string& filename = "copy");

#endif
