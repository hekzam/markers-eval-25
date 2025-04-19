#ifndef DRAW_HELPER_H
#define DRAW_HELPER_H

/**
 * @file draw_helper.h
 * @brief Utilitaires pour le dessin et la sauvegarde d'images de débogage pour les codes-barres
 */

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <common.h>

/**
 * @brief Fonctions utilitaires pour dessiner et sauvegarder des images de débogage
 */

/**
 * @brief Dessine les contours des codes-barres détectés sur l'image
 * @param barcodes Liste des codes-barres détectés avec leurs coordonnées
 * @param debug_img Image sur laquelle dessiner les contours (modifiée par la fonction)
 */
void draw_qrcode(std::vector<DetectedBarcode>& barcodes, cv::Mat& debug_img);

/**
 * @brief Sauvegarde l'image de débogage dans le répertoire spécifié
 * @param debug_img Image à sauvegarder
 * @param output_dir Chemin du répertoire de sortie
 * @param output_img_path_fname Nom du fichier de sortie (sera préfixé par "cal-debug-")
 */
void save_debug_img(cv::Mat debug_img, std::filesystem::path output_dir, std::filesystem::path output_img_path_fname);

#endif