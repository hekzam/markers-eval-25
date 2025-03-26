#ifndef TYPST_INTERFACE_H
#define TYPST_INTERFACE_H

#include <string>

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

// Function to create a copy with markers
bool create_copy(int encoded_marker_size, int fiducial_marker_size, int stroke_width, int marker_margin, int nb_copies,
                 int duplex_printing, int marker_config, int grey_level, int header_marker);

#endif
