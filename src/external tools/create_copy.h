#ifndef TYPST_INTERFACE_H
#define TYPST_INTERFACE_H

#include <string>

/**
 * Génère une copie de marqueurs fiduciaux et encodés
 *
 * @param encoded_marker_size Taille des marqueurs encodés
 * @param fiducial_marker_size Taille des marqueurs fiduciaux
 * @param stroke_width Largeur du trait des marqueurs
 * @param marker_margin Marge autour des marqueurs
 * @param nb_copies Nombre de copies à générer
 * @param duplex_printing 0: impression recto, 1: impression recto-verso
 * @param marker_config Configuration des marqueurs
 * @param grey_level Niveau de gris (0: noir, 255: blanc)
 * @return true si la génération a réussi, false sinon
 */
bool create_copy(int encoded_marker_size = 15, int fiducial_marker_size = 10, int stroke_width = 2,
                 int marker_margin = 3, int nb_copies = 1, int duplex_printing = 0, int marker_config = 10,
                 int grey_level = 100);

#endif
