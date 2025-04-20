#ifndef STRING_HELPER_H
#define STRING_HELPER_H

/**
 * @file string_helper.h
 * @brief Module d'utilitaires pour la manipulation des chaînes de caractères
 *
 * Ce module fournit des fonctions pour faciliter le traitement et l'analyse
 * des chaînes de caractères, comme la division d'une chaîne selon un délimiteur
 * ou la vérification de préfixe.
 */

#include <vector>
#include <string>

/**
 * @brief Divise une chaîne de caractères en plusieurs sous-chaînes selon un délimiteur
 *
 * Cette fonction parcourt la chaîne d'entrée et la découpe en sous-chaînes
 * chaque fois que le délimiteur est rencontré.
 *
 * @param s Chaîne de caractères à découper
 * @param delimiter Chaîne délimiteur qui indique où effectuer la division
 * @return std::vector<std::string> Vecteur contenant les sous-chaînes résultantes
 */
std::vector<std::string> split(std::string s, std::string delimiter);

/**
 * @brief Vérifie si une chaîne commence par un préfixe spécifié
 *
 * @param s Chaîne à vérifier
 * @param prefix Préfixe à rechercher au début de la chaîne
 * @return bool Retourne vrai si la chaîne commence par le préfixe, faux sinon
 */
bool starts_with(std::string s, std::string prefix);

#endif