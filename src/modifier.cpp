/**
 * @file modifier.cpp
 * @brief Utilitaire de démonstration pour la rotation d'image
 * 
 * Cet outil permet de charger une image depuis un chemin spécifié,
 * lui appliquer une rotation de 45 degrés autour de son centre,
 * et enregistrer le résultat dans un nouveau fichier.
 */

#include <iostream>
#include <common.h>
#include "utils/math_utils.h"

/**
 * @brief Point d'entrée du programme de modification d'image
 * 
 * Ce programme charge une image, applique une rotation de 45 degrés
 * autour du centre de l'image, et enregistre le résultat dans le fichier
 * "calibrated_img.png" dans le répertoire courant.
 * 
 * @param argc Nombre d'arguments passés au programme
 * @param argv Tableau des arguments. argv[1] doit contenir le chemin de l'image à traiter
 * @return int Code de retour (0 en cas de succès, 1 si arguments insuffisants)
 */
int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }
    std::string image_path = argv[1];
    cv::Mat img = cv::imread(image_path);
    cv::Mat calibrated_img = img.clone();
    
    // Création et application de la matrice de transformation pour une rotation de 45 degrés
    cv::Mat identity = cv::Mat::eye(3, 3, CV_32F);
    identity *= rotate_center(45, img.cols / 2, img.rows / 2);
    print_mat(identity);
    identity *= translate(0, 0);
    print_mat(identity);
    
    // Extraction de la sous-matrice 3x2 pour warpAffine
    identity = identity(cv::Rect(0, 0, 3, 2));
    print_mat(identity);
    
    // Application de la transformation et sauvegarde du résultat
    cv::warpAffine(img, calibrated_img, identity, calibrated_img.size(), cv::INTER_LINEAR);
    cv::imwrite("calibrated_img.png", calibrated_img);
    return 0;
}
