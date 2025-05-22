#ifndef LIMITE_BENCH_H
#define LIMITE_BENCH_H

/**
 * @brief Fonction de benchmark pour évaluer la robustesse de détection des marqueurs sous diverses conditions de dégradation
 *
 * Cette fonction effectue une évaluation systématique de la performance de détection des marqueurs
 * en appliquant des transformations et dégradations progressives aux images de test.
 * Elle teste différentes conditions incluant:
 * - Simulation des effets d'impression
 * - Bruit sel et poivre
 * - Bruit gaussien
 * - Modifications de contraste et luminosité
 * - Taches d'encre
 * - Rotation d'image
 * - Translation d'image
 * - Artefacts de compression JPEG
 * - Variations de taille des marqueurs
 * - Variations de niveaux de gris
 * - Changements de résolution (DPI)
 * - Variations de largeur de trait
 *
 * Les résultats sont enregistrés dans un fichier CSV pour analyse, incluant le temps d'analyse,
 * le taux de réussite et les mesures d'erreur de précision.
 *
 * @param config Une map non ordonnée contenant les paramètres de configuration incluant:
 *               - warmup-iterations: Nombre d'itérations d'échauffement avant le benchmark
 *               - nb-copies: Nombre de copies à créer pour chaque condition de test
 *               - encoded-marker-size: Taille des marqueurs encodés en pixels
 *               - unencoded-marker-size: Taille des marqueurs non encodés en pixels
 *               - header-marker-size: Taille des marqueurs d'en-tête en pixels
 *               - grey-level: Valeur de niveau de gris pour les marqueurs
 *               - dpi: Résolution en points par pouce
 *               - seed: Graine pour le générateur de nombres aléatoires
 *               - marker-config: Chaîne de configuration pour la génération de marqueurs
 *               - parser-type: Type d'analyseur à utiliser (optionnel, par défaut ZXING)
 *               - csv-mode: Mode d'ajout ou d'écrasement pour la sortie CSV (optionnel)
 *               - csv-filename: Nom du fichier CSV de sortie (optionnel)
 */
void limite_bench(const std::unordered_map<std::string, Config>& config);

#endif