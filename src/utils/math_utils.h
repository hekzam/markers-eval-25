#ifndef math_utils_H
#define math_utils_H

/**
 * @file math_utils.h
 * @brief Utilitaires mathématiques pour la manipulation de coordonnées et de transformations géométriques
 */

/**
 * @brief Redimensionne une coordonnée d'une image source vers une image destination
 * @param src_coord Coordonnée à redimensionner
 * @param src_img_size Taille de l'image source (largeur, hauteur)
 * @param dst_img_size Taille de l'image destination (largeur, hauteur)
 * @return Coordonnée redimensionnée dans l'image destination
 */
cv::Point2f coord_scale(const cv::Point2f& src_coord, const cv::Point2f& src_img_size, const cv::Point2f& dst_img_size);

/**
 * @brief Convertit un ensemble de points en coordonnées flottantes vers des coordonnées raster (entiers)
 * @param vec_points Vecteur de points en coordonnées flottantes
 * @param src_img_size Taille de l'image source (largeur, hauteur)
 * @param dst_img_size Taille de l'image destination (largeur, hauteur)
 * @return Vecteur de points en coordonnées raster (entiers)
 */
std::vector<cv::Point> convert_to_raster(const std::vector<cv::Point2f>& vec_points, const cv::Point2f& src_img_size,
                                         const cv::Point2f& dst_img_size);

/**
 * @brief Calcule le centre d'un ensemble de points formant un quadrilatère
 * @param bounding_box Vecteur de points définissant le quadrilatère
 * @return Point central du quadrilatère
 */
cv::Point2f center_of_box(std::vector<cv::Point2f> bounding_box);

/**
 * @brief Calcule l'angle formé par trois points (a, b, c) avec b comme sommet
 * @param a Premier point
 * @param b Point de sommet (vertex)
 * @param c Troisième point
 * @return Angle en radians
 */
float angle(cv::Point2f a, cv::Point2f b, cv::Point2f c);

/**
 * @brief Trouve les autres coins d'un quadrilatère à partir d'un coin connu
 * @param points Liste de tous les points candidats
 * @param corner_points Vecteur qui contiendra les coins trouvés (modifié par la fonction)
 * @param corner_barcode Point de coin déjà connu
 * @return Un masque binaire indiquant quels coins ont été trouvés
 */
int found_other_point(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_points,
                      cv::Point2f corner_barcode);

/**
 * @brief Compte le nombre de bits actifs dans un masque
 * @param mask Masque binaire à évaluer
 * @return Nombre de bits actifs
 */
int sum_mask(int mask);

/**
 * @brief Crée une matrice de translation
 * @param x Translation en x
 * @param y Translation en y
 * @return Matrice de transformation 3x3
 */
cv::Mat translate(float x, float y);

/**
 * @brief Crée une matrice de rotation autour de l'origine
 * @param angle Angle de rotation en degrés
 * @return Matrice de transformation 3x3
 */
cv::Mat rotate(float angle);

/**
 * @brief Crée une matrice de rotation autour d'un point spécifique
 * @param angle Angle de rotation en degrés
 * @param cx Coordonnée x du centre de rotation
 * @param cy Coordonnée y du centre de rotation
 * @return Matrice de transformation 3x3
 */
cv::Mat rotate_center(float angle, float cx, float cy);

/**
 * @brief Affiche une matrice sur la sortie standard
 * @param mat Matrice à afficher
 */
void print_mat(cv::Mat mat);
double percentage_to_offset(int depth, double percentage);
double percentage_to_dispersion(int depth, double percentage);
template<typename In, typename Out>
Out normalize_range(In v, In in_min, In in_max, Out out_min, Out out_max)
{
    // Calcul du ratio (en double pour conserver précision)
    double ratio = 0.0;
    if (in_max != in_min) {
        ratio = static_cast<double>(v - in_min)
              / static_cast<double>(in_max - in_min);
    }
    // Clamp dans [0,1]
    ratio = std::clamp(ratio, 0.0, 1.0);

    // Passage dans la nouvelle échelle
    double mapped = static_cast<double>(out_min)
                  + ratio * (static_cast<double>(out_max) - static_cast<double>(out_min));

    // Si sortie entière, on arrondit
    if constexpr (std::is_integral_v<Out>) {
        return static_cast<Out>(std::round(mapped));
    } else {
        return static_cast<Out>(mapped);
    }
}


#endif



