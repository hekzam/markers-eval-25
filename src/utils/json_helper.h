#ifndef JSON_HELPER_H
#define JSON_HELPER_H

/**
 * @file json_helper.h
 * @brief Utilitaires pour le traitement des données JSON et la conversion en structures de données
 */

#include <common.h>
#include <vector>
#include <memory>

/**
 * @brief Convertit le contenu JSON en une liste d'objets AtomicBox
 * @param content Contenu JSON à convertir
 * @return Vecteur d'objets AtomicBox partagés
 */
std::vector<std::shared_ptr<AtomicBox>> json_to_atomicBox(const json& content);

/**
 * @brief Analyse une chaîne de caractères pour en extraire les métadonnées
 * @param content Chaîne de caractères contenant les métadonnées au format CSV
 * @return Structure Metadata contenant les informations extraites
 */
Metadata parse_metadata(std::string content);

#endif // JSON_HELPER_H
