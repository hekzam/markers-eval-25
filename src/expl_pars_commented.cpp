// Bibliothèques standards C++ utilisées pour la gestion des entrées/sorties, des fichiers et des opérations de base
#include <iostream>      // Permet l'affichage et la lecture des entrées/sorties standard (std::cout, std::cin, std::cerr)
#include <fstream>       // Fournit des fonctionnalités pour la lecture et l'écriture de fichiers
#include <stdio.h>       // Fonctions standard de la bibliothèque C (printf, fopen, fclose, etc.)
#include <string.h>      // Fonctions de manipulation de chaînes de caractères en C (strlen, strcpy, etc.)
#include <cmath>         // Fonctions mathématiques courantes (sqrt, pow, abs, etc.)
#include <filesystem>    // Gestion des fichiers et des répertoires (création, suppression, navigation)

// Bibliothèques OpenCV pour le traitement d'images
#include <opencv2/imgcodecs.hpp>  // Gestion des fichiers image (lecture et écriture de PNG, JPEG, etc.)
#include <opencv2/imgproc.hpp>    // Fonctions de traitement d'image (filtrage, transformation, seuillage, etc.)
#include <opencv2/objdetect.hpp>  // Détection d'objets (ex. : reconnaissance faciale, détection de formes)

// Bibliothèque ZXing pour la lecture de codes-barres et de QR codes
#include <ZXing/ReadBarcode.h>    // Interface pour la détection et la lecture des codes-barres

// Inclusion de fichiers spécifiques au projet
#include <common.h>               // Fichier d'en-tête contenant des définitions et fonctions communes au projet
#include "utils/json_helper.h"    // Utilitaires pour la gestion et la manipulation des fichiers JSON

// Gestion de la mémoire dynamique avec des pointeurs intelligents
#include <memory>                 // Fournit std::shared_ptr et std::unique_ptr pour la gestion automatique de la mémoire



/**
 * @brief Énumération des coins d'une image sous forme de flags binaires.
 *
 * Chaque valeur correspond à un coin spécifique et peut être combinée via des opérations binaires
 * pour représenter plusieurs coins simultanément.
 */
enum CornerBF {
    TOP_LEFT_BF = 0x01,       ///< Coin supérieur gauche
    TOP_RIGHT_BF = 0x02,      ///< Coin supérieur droit
    BOTTOM_LEFT_BF = 0x04,    ///< Coin inférieur gauche
    BOTTOM_RIGHT_BF = 0x08    ///< Coin inférieur droit
};

/**
 * @brief Structure contenant les métadonnées associées à un élément.
 *
 * Cette structure regroupe des informations utiles telles que :
 * - `name` : Nom associé à l'élément.
 * - `page` : Numéro de page sur laquelle l'élément est situé.
 * - `id` : Identifiant unique de l'élément.
 */
struct Metadata {
    std::string name; ///< Nom de l'élément
    int page;         ///< Numéro de page associé
    int id;           ///< Identifiant unique
};

/**
 * @brief Structure représentant un code-barres détecté.
 *
 * Cette structure contient :
 * - `content` : Chaîne de caractères représentant le contenu du code-barres détecté.
 * - `bounding_box` : Coordonnées des coins délimitant la zone englobante du code-barres.
 */
struct DetectedBarcode {
    std::string content;               ///< Contenu du code-barres détecté
    std::vector<cv::Point2f> bounding_box; ///< Boîte englobante définie par des points en 2D
};


/**
 * @brief Analyse et extrait les informations des boîtes atomiques à partir d'un objet JSON.
 *
 * Cette fonction parcourt les éléments du JSON fourni et, pour chaque paire clé/valeur,
 * crée une instance d'AtomicBox en extrayant l'identifiant, la page, ainsi que les coordonnées
 * (x, y) et dimensions (width, height). Chaque boîte est ensuite ajoutée au vecteur 'boxes'.
 *
 * @param content Le JSON contenant les données des boîtes.
 * @param boxes Le vecteur dans lequel les boîtes extraites seront stockées.
 */
void parse_atomic_boxes(const json& content, std::vector<AtomicBox>& boxes) {
    // Parcourt chaque élément du JSON en récupérant la clé et la valeur associée
    for (const auto& [key, value] : content.items()) {
        AtomicBox box;  // Crée une instance de AtomicBox pour stocker les données

        // Assigne l'ID de la boîte en utilisant la clé du JSON
        box.id = key;
        
        // Extrait et stocke les informations de position et de dimension de la boîte
        box.page = value["page"];      // Numéro de la page où se trouve la boîte
        box.x = value["x"];            // Position en X de la boîte
        box.y = value["y"];            // Position en Y de la boîte
        box.width = value["width"];     // Largeur de la boîte
        box.height = value["height"];   // Hauteur de la boîte

        // Ajoute la boîte construite à la liste des boîtes
        boxes.emplace_back(box);
    }
}

/**
 * @brief Classe et différencie les boîtes atomiques en différentes catégories.
 *
 * Cette fonction prend une liste de boîtes atomiques et les répartit en plusieurs catégories :
 * - `markers` : Contient les boîtes identifiées comme marqueurs.
 * - `corner_markers` : Contient les marqueurs de coins (haut gauche, haut droit, bas gauche, bas droit).
 * - `user_boxes_per_page` : Liste des boîtes classées par page.
 *
 * @param boxes Vecteur contenant toutes les boîtes atomiques à traiter.
 * @param markers Vecteur où seront stockés les boîtes identifiées comme marqueurs.
 * @param corner_markers Tableau contenant les marqueurs de coins classés par position.
 * @param user_boxes_per_page Vecteur 2D stockant les boîtes utilisateur, classées par page.
 *
 * @throws std::invalid_argument si certains marqueurs de coins sont manquants.
 */
void differentiate_atomic_boxes(std::vector<std::shared_ptr<AtomicBox>>& boxes,
                                std::vector<std::shared_ptr<AtomicBox>>& markers,
                                std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                std::vector<std::vector<std::shared_ptr<AtomicBox>>>& user_boxes_per_page) {
    // Nettoyage des conteneurs pour éviter toute accumulation de données précédentes
    markers.clear();
    corner_markers.resize(4); // 4 coins
    user_boxes_per_page.clear();

    // Vérification si le vecteur de boîtes est vide, dans ce cas on quitte la fonction
    if (boxes.empty())
        return;

    int max_page = 1;

    // Déterminer la page maximale parmi toutes les boîtes
    for (const auto& box : boxes) {
        max_page = std::max(max_page, box->page);
    }
    user_boxes_per_page.resize(max_page); // Allocation du vecteur en fonction du nombre de pages détectées

    // Parcours des boîtes pour les classer en marqueurs et boîtes utilisateur
    for (auto box : boxes) {
        if (box->id.find("marker barcode ") == 0) {
            markers.emplace_back(box); // Ajout aux marqueurs si l'ID commence par "marker barcode"
        } else {
            user_boxes_per_page.at(box->page - 1).emplace_back(box); // Classement des boîtes utilisateur par page
        }
    }

    int corner_mask = 0; // Bitmask pour suivre les coins détectés

    // Identification et stockage des marqueurs de coins
    for (auto box : markers) {
        int corner = -1;
        if (box->id.rfind("marker barcode tl", 0) == 0) // Coin supérieur gauche
            corner = TOP_LEFT;
        else if (box->id.rfind("marker barcode tr", 0) == 0) // Coin supérieur droit
            corner = TOP_RIGHT;
        else if (box->id.rfind("marker barcode bl", 0) == 0) // Coin inférieur gauche
            corner = BOTTOM_LEFT;
        else if (box->id.rfind("marker barcode br", 0) == 0) // Coin inférieur droit
            corner = BOTTOM_RIGHT;

        // Si un marqueur de coin a été identifié, on l'ajoute à la liste des coins et on met à jour le mask
        if (corner != -1) {
            corner_markers[corner] = box;
            corner_mask |= (1 << corner);
        }
    }

    // Vérification que tous les marqueurs de coin sont bien présents, sinon une exception est levée
    if (corner_mask != (TOP_LEFT_BF | TOP_RIGHT_BF | BOTTOM_LEFT_BF | BOTTOM_RIGHT_BF))
        throw std::invalid_argument("some corner markers are missing in the atomic box JSON description");
}

/**
 * @brief Effectue une transformation d'échelle des coordonnées d'un point entre deux images de tailles différentes.
 *
 * Cette fonction permet de redimensionner un point d'une image source à une image de destination,
 * en conservant la proportionnalité entre les deux espaces.
 *
 * @param src_coord Coordonnée du point dans l'image source.
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image de destination (largeur, hauteur).
 * @return cv::Point2f Coordonnée transformée dans l'image de destination.
 */
cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size,
                        const cv::Point2f& dst_img_size) {
    return cv::Point2f{
        (src_coord.x / src_img_size.x) * dst_img_size.x, // Mise à l'échelle en X
        (src_coord.y / src_img_size.y) * dst_img_size.y  // Mise à l'échelle en Y
    };
}

/**
 * @brief Calcule les centres des marqueurs de coins après mise à l'échelle entre deux espaces d'images.
 *
 * Cette fonction prend en entrée une liste de marqueurs de coins et leur applique une transformation
 * d'échelle afin d'obtenir leurs centres dans l'espace de l'image de destination.
 *
 * @param corner_markers Vecteur contenant 4 marqueurs de coins sous forme de `shared_ptr<AtomicBox>`.
 * @param src_img_size Taille de l'image source (largeur, hauteur).
 * @param dst_img_size Taille de l'image de destination (largeur, hauteur).
 * @return std::vector<cv::Point2f> Vecteur contenant les centres des marqueurs après transformation.
 */
std::vector<cv::Point2f> calculate_center_of_marker(const std::vector<std::shared_ptr<AtomicBox>>& corner_markers,
                                                    const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size) {
    std::vector<cv::Point2f> corner_points(4); // Initialisation du tableau des points centraux des marqueurs

    for (int corner = 0; corner < 4; ++corner) {
        auto marker = corner_markers[corner]; // Récupération du marqueur correspondant au coin

        // Définition des coins de la boîte englobante du marqueur
        const std::vector<cv::Point2f> marker_bounding_box = {
            cv::Point2f{ marker->x, marker->y },                                   // Haut-gauche
            cv::Point2f{ marker->x + marker->width, marker->y },                   // Haut-droit
            cv::Point2f{ marker->x + marker->width, marker->y + marker->height },  // Bas-droit
            cv::Point2f{ marker->x, marker->y + marker->height }                    // Bas-gauche
        };

        // Calcul du centre du marqueur en moyennant les coordonnées de ses coins
        cv::Mat mean_mat;
        cv::reduce(marker_bounding_box, mean_mat, 1, cv::REDUCE_AVG);
        cv::Point2f mean_point{ mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1) };

        // Mise à l'échelle du centre du marqueur vers l'espace de l'image de destination
        corner_points[corner] = coord_scale(mean_point, src_img_size, dst_img_size);
    }
    
    return corner_points; // Retourne la liste des centres des marqueurs transformés
}


/**
 * @brief Analyse une chaîne de métadonnées et extrait le numéro de copie et le numéro de page.
 *
 * Cette fonction suppose que le format de la chaîne est "hzbl,COPYNUMBER,PAGENUMBER".
 * Elle extrait les valeurs numériques en utilisant `strtol` pour les convertir en entiers.
 *
 * @param content Chaîne contenant les métadonnées sous forme "hzbl,COPYNUMBER,PAGENUMBER".
 * @return Metadata Objet contenant les informations extraites (numéro de copie et numéro de page).
 */
Metadata parse_metadata(std::string content) {
    const char* bl_qrcode_str = content.c_str(); // Convertit la chaîne en C-string pour l'analyse
    char* parse_ptr = nullptr;

    // Extrait le numéro de copie après "hzbl," (position 5)
    int copy = strtol(bl_qrcode_str + 5, &parse_ptr, 10);

    // Extrait le numéro de page après la virgule suivante
    int page = strtol(parse_ptr + 1, NULL, 10);

    Metadata metadata;
    metadata.name = ""; // Nom non utilisé ici
    metadata.page = page;
    metadata.id = copy;

    return metadata; // Retourne les métadonnées extraites
}

/**
 * @brief Calcule la transformation affine entre les points détectés et les points attendus.
 *
 * Cette fonction extrait les coordonnées des coins détectés et les associe aux coordonnées
 * des coins attendus pour calculer une transformation affine permettant d'aligner les deux jeux de points.
 * 
 * @param found_corner_mask Masque indiquant quels coins ont été détectés.
 * @param expected_corner_points Liste des coins attendus dans l'image de référence.
 * @param found_corner_points Liste des coins réellement détectés.
 * @param affine_transform Matrice de transformation affine résultante.
 * 
 * @throws std::invalid_argument si moins de trois coins sont détectés.
 */
void get_affine_transform(int found_corner_mask, const std::vector<cv::Point2f>& expected_corner_points,
                          const std::vector<cv::Point2f>& found_corner_points, cv::Mat& affine_transform) {
    int nb_found = 0; // Nombre de coins détectés
    std::vector<cv::Point2f> src, dst;
    src.reserve(3); // Allocation pour 3 points minimum
    dst.reserve(3);

    // Itération sur les quatre coins possibles pour trouver les trois premiers détectés
    for (int corner = 0; corner < 4; ++corner) {
        if ((1 << corner) & found_corner_mask) { // Vérifie si le coin est marqué comme détecté
            src.emplace_back(found_corner_points[corner]);     // Ajoute le point détecté
            dst.emplace_back(expected_corner_points[corner]);  // Ajoute le point attendu

            nb_found += 1;
            if (nb_found >= 3) // Stoppe l'extraction dès que trois coins sont collectés
                break;
        }
    }

    // Vérification du nombre de coins détectés pour garantir un calcul valide
    if (nb_found != 3)
        throw std::invalid_argument("only " + std::to_string(nb_found) + " corners were found (3 or more required)");

    // Calcul de la transformation affine à partir des points correspondants
    affine_transform = cv::getAffineTransform(src, dst);
}


/**
 * @brief Identifie les codes-barres des coins d'une image et extrait leurs positions.
 *
 * Cette fonction parcourt une liste de codes-barres détectés et identifie ceux correspondant aux coins
 * (haut-gauche, haut-droit, bas-gauche, bas-droit) en analysant leur contenu.
 * Les coordonnées moyennes des boîtes englobantes des codes-barres détectés sont calculées et stockées.
 *
 * @param barcodes Liste des codes-barres détectés dans l'image.
 * @param content_hash Chaîne de hachage utilisée pour vérifier l'authenticité du code-barres du coin bas-droit.
 * @param corner_points Vecteur de points où seront stockées les coordonnées des coins identifiés.
 * @param corner_barcodes Vecteur de pointeurs vers les `DetectedBarcode` identifiés comme marqueurs de coins.
 * @return int Un masque binaire indiquant quels coins ont été trouvés.
 */
int identify_corner_barcodes(std::vector<DetectedBarcode>& barcodes, const std::string& content_hash,
                             std::vector<cv::Point2f>& corner_points, std::vector<DetectedBarcode*>& corner_barcodes) {
    corner_points.resize(4);  // Initialise l'espace pour 4 coins
    corner_barcodes.resize(4);
    int found_mask = 0x00;  // Masque binaire indiquant quels coins ont été détectés

    for (auto& barcode : barcodes) {
        // Vérifie si la chaîne contient au moins 4 caractères pour identifier un marqueur
        if (barcode.content.size() < 4)
            continue;

        const char* s = barcode.content.c_str();
        int pos_found = 0;

        // Vérifie si le code commence par "hz"
        uint16_t hz = (s[0] << 8) + s[1];
        if (hz == ('h' << 8) + 'z') {
            // Extraction des deux caractères suivants pour identifier le coin
            uint16_t xy = (s[2] << 8) + s[3];
            switch (xy) {
                case ('t' << 8) + 'l':
                    pos_found = TOP_LEFT;
                    break;
                case ('t' << 8) + 'r':
                    pos_found = TOP_RIGHT;
                    break;
                case ('b' << 8) + 'l':
                    pos_found = BOTTOM_LEFT;
                    break;
                case ('b' << 8) + 'r': {
                    // Vérifie si le contenu du code-barres inclut le hachage attendu pour le bas-droit
                    if (strstr(s, content_hash.c_str()) == NULL)
                        continue;
                    pos_found = BOTTOM_RIGHT;
                } break;
                default:
                    continue;
            }

            // Calcul du centre de la boîte englobante du code-barres
            cv::Mat mean_mat;
            cv::reduce(barcode.bounding_box, mean_mat, 1, cv::REDUCE_AVG);
            corner_points[pos_found] = cv::Point2f(mean_mat.at<float>(0, 0), mean_mat.at<float>(0, 1));

            // Associe le code-barres détecté au coin correspondant
            corner_barcodes[pos_found] = &barcode;
            int pos_found_bf = 1 << pos_found;

            // Mise à jour du masque binaire indiquant les coins trouvés
            found_mask |= pos_found_bf;
        }
    }

    return found_mask;
}

/**
 * @brief Détecte les codes-barres présents dans une image.
 *
 * Cette fonction utilise la bibliothèque ZXing pour détecter les codes-barres dans une image en niveaux de gris.
 * Elle extrait le contenu textuel ainsi que les coordonnées des coins des boîtes englobantes des codes-barres détectés.
 *
 * @param img Image d'entrée en niveaux de gris (CV_8U).
 * @return std::vector<DetectedBarcode> Liste des codes-barres détectés avec leurs informations associées.
 * @throws std::invalid_argument Si l'image n'est pas en niveaux de gris (CV_8U).
 */
std::vector<DetectedBarcode> detect_barcodes(cv::Mat img) {
    std::vector<DetectedBarcode> barcodes = {};

    // Vérification du type de l'image (doit être en niveaux de gris)
    if (img.type() != CV_8U)
        throw std::invalid_argument(
            "img has type != CV_8U while it should contain luminance information on 8-bit unsigned integers");

    // Vérifie que l'image est suffisamment grande pour contenir des codes-barres
    if (img.cols < 2 || img.rows < 2)
        return {};

    // Conversion de l'image OpenCV en format ZXing pour analyse des codes-barres
    auto iv = ZXing::ImageView(reinterpret_cast<const uint8_t*>(img.ptr()), img.cols, img.rows, ZXing::ImageFormat::Lum);
    auto z_barcodes = ZXing::ReadBarcodes(iv);

    // Extraction des codes-barres détectés
    for (const auto& b : z_barcodes) {
        DetectedBarcode barcode;
        barcode.content = b.text();  // Stocke le texte du code-barres

        // Récupère les coins de la boîte englobante du code-barres
        for (int j = 0; j < 4; ++j) {
            const auto& p = b.position()[j];
            barcode.bounding_box.emplace_back(cv::Point2f(p.x, p.y));
        }

        barcodes.emplace_back(barcode);
    }

    return barcodes; // Retourne la liste des codes-barres détectés
}


/**
 * @brief Programme principal du parseur d'images pour détecter et extraire des boîtes atomiques.
 *
 * Ce programme prend en entrée un répertoire de sortie, un fichier JSON contenant des informations
 * sur les boîtes atomiques, et une ou plusieurs images. Il réalise les étapes suivantes :
 * 1. Création des répertoires nécessaires.
 * 2. Lecture et parsing du fichier JSON des boîtes atomiques.
 * 3. Classification des boîtes en marqueurs et boîtes utilisateur.
 * 4. Chargement et traitement de chaque image :
 *    - Détection des codes-barres et identification des marqueurs de coins.
 *    - Calcul de la transformation affine pour aligner l'image.
 *    - Extraction et sauvegarde des régions d'intérêt correspondant aux boîtes atomiques.
 *    - Génération d'une image annotée avec les boîtes détectées.
 *
 * @param argc Nombre d'arguments passés au programme.
 * @param argv Tableau des arguments (OUTPUT_DIR, ATOMIC_BOXES, IMAGE...).
 * @return int Code de retour du programme (0 si succès, 1 en cas d'erreur).
 */
int main(int argc, char* argv[]) {
    // Vérification des arguments
    if (argc < 4) {
        fprintf(stderr, "usage: parser OUTPUT_DIR ATOMIC_BOXES IMAGE...\n");
        return 1;
    }

    // Création des répertoires de sortie si nécessaire
    std::filesystem::path output_dir{ argv[1] };
    std::filesystem::create_directories(output_dir);

    std::filesystem::path subimg_output_dir = output_dir.string() + std::string("/subimg");
    std::filesystem::create_directories(subimg_output_dir);

    // Lecture du fichier JSON contenant les informations sur les boîtes atomiques
    std::ifstream atomic_boxes_file(argv[2]);
    if (!atomic_boxes_file.is_open()) {
        fprintf(stderr, "could not open file '%s'\n", argv[2]);
        return 1;
    }
    json atomic_boxes_json;
    try {
        atomic_boxes_json = json::parse(atomic_boxes_file);
    } catch (const json::exception& e) {
        fprintf(stderr, "could not json parse file '%s': %s", argv[2], e.what());
        return 1;
    }

    // Chaîne de hachage attendue pour vérifier l'intégrité des codes-barres détectés
    const std::string expected_content_hash = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=";

    // Conversion du JSON en objets AtomicBox et classification des boîtes
    auto atomic_boxes = json_to_atomicBox(atomic_boxes_json);
    std::vector<std::shared_ptr<AtomicBox>> markers;
    std::vector<std::shared_ptr<AtomicBox>> corner_markers;
    std::vector<std::vector<std::shared_ptr<AtomicBox>>> user_boxes_per_page;
    differentiate_atomic_boxes(atomic_boxes, markers, corner_markers, user_boxes_per_page);

    // Définition de la taille de l'image source (supposée être en format A4)
    const cv::Point2f src_img_size{ 210, 297 }; // TODO: rendre cela dynamique

    // Traitement de chaque image passée en argument
    for (int i = 3; i < argc; ++i) {
        // Chargement de l'image en niveaux de gris
        cv::Mat img = cv::imread(argv[i], cv::IMREAD_GRAYSCALE);
        const cv::Point2f dst_img_size(img.cols, img.rows);

        // Calcul des positions des coins après mise à l'échelle
        std::vector<cv::Point2f> dst_corner_points = calculate_center_of_marker(corner_markers, src_img_size, dst_img_size);

        // Détection des codes-barres dans l'image
        auto barcodes = detect_barcodes(img);

        // Identification des codes-barres des coins
        std::vector<cv::Point2f> corner_points;
        std::vector<DetectedBarcode*> corner_barcodes;
        int found_corner_mask = identify_corner_barcodes(barcodes, expected_content_hash, corner_points, corner_barcodes);

        // Extraction des métadonnées de l'examen depuis le code-barres bas-gauche
        auto metadata = parse_metadata(corner_barcodes[BOTTOM_LEFT]->content);

        // Calcul de la transformation affine pour aligner l'image
        cv::Mat affine_transform;
        get_affine_transform(found_corner_mask, dst_corner_points, corner_points, affine_transform);

        // Appliquer la transformation pour obtenir une image calibrée
        cv::Mat calibrated_img;
        warpAffine(img, calibrated_img, affine_transform, img.size(), cv::INTER_LINEAR);

        // Conversion en couleur pour annotation
        cv::Mat calibrated_img_col;
        cv::cvtColor(calibrated_img, calibrated_img_col, cv::COLOR_GRAY2BGR);

        // Extraction des boîtes utilisateur et sauvegarde des sous-images
        for (auto box : user_boxes_per_page[metadata.page - 1]) {
            // Définition des coins de la boîte
            const std::vector<cv::Point2f> vec_box = { 
                { box->x, box->y }, 
                { box->x + box->width, box->y },
                { box->x + box->width, box->y + box->height },
                { box->x, box->y + box->height } 
            };

            // Mise à l'échelle et conversion en coordonnées d'image
            std::vector<cv::Point> raster_box;
            int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
            for (const auto& pt : vec_box) {
                auto scaled = coord_scale(pt, src_img_size, dst_img_size);
                int x = round(scaled.x), y = round(scaled.y);
                min_x = std::min(min_x, x);
                max_x = std::max(max_x, x);
                min_y = std::min(min_y, y);
                max_y = std::max(max_y, y);
                raster_box.emplace_back(cv::Point(x, y));
            }

            // Extraction de la sous-image correspondant à la boîte détectée
            cv::Mat subimg = calibrated_img(cv::Range(min_y, max_y), cv::Range(min_x, max_x));
            char* output_img_fname = nullptr;
            int nb = asprintf(&output_img_fname, "%s/subimg/raw-%d-%s.png", output_dir.c_str(), metadata.id, box->id.c_str());
            cv::imwrite(output_img_fname, subimg);
            free(output_img_fname);

            // Annotation de l'image calibrée avec les contours détectés
            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(0, 0, 255), 2);
        }

        // Annotation des marqueurs de coins détectés en bleu
        for (auto box : corner_markers) {
            if (strncmp("marker barcode br", box->id.c_str(), 17) == 0)
                break;

            std::vector<cv::Point> raster_box;
            for (const auto& pt : { 
                cv::Point2f{ box->x, box->y }, 
                cv::Point2f{ box->x + box->width, box->y },
                cv::Point2f{ box->x + box->width, box->y + box->height },
                cv::Point2f{ box->x, box->y + box->height } }) {
                auto scaled = coord_scale(pt, src_img_size, dst_img_size);
                raster_box.emplace_back(cv::Point(round(scaled.x), round(scaled.y)));
            }

            cv::polylines(calibrated_img_col, raster_box, true, cv::Scalar(255, 0, 0), 2);
        }

        // Sauvegarde de l'image annotée
        std::filesystem::path input_img_path{ argv[i] };
        std::filesystem::path output_img_path_fname = input_img_path.filename().replace_extension(".png");
        char* output_img_fname = nullptr;
        int nb = asprintf(&output_img_fname, "%s/cal-%s", output_dir.c_str(), output_img_path_fname.c_str());
        cv::imwrite(output_img_fname, calibrated_img_col);
        free(output_img_fname);
    }

    return 0;
}

