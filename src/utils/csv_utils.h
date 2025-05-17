#ifndef CVS_UTILS_H
#define CVS_UTILS_H

/**
 * @file csv_utils.h
 * @brief Module utilitaire pour la manipulation de fichiers CSV.
 *
 * Ce module fournit des classes et fonctions pour faciliter la création,
 * la modification et la lecture de fichiers au format CSV (Comma-Separated Values).
 */

#include <tuple>
#include <string>
#include <fstream>
#include <filesystem>

/**
 * @brief Mode d'ouverture du fichier CSV
 *
 * Définit le comportement lors de l'ouverture d'un fichier CSV existant.
 */
enum class CsvMode { APPEND, OVERWRITE };

/**
 * @brief Classe générique pour la gestion de fichiers CSV
 *
 * Permet de créer, modifier et écrire des données dans un fichier CSV.
 * Supporte un nombre variable de colonnes via les templates variadic.
 *
 * @tparam Args Types des données à écrire dans les colonnes du CSV
 */
template <class... Args> class Csv {
  public:
    /**
     * @brief Construit un nouvel objet CSV
     *
     * @param filename Nom/chemin du fichier CSV
     * @param headers En-têtes des colonnes du fichier
     * @param mode Mode d'ouverture du fichier (ajout ou écrasement)
     * @throw std::runtime_error Si l'ouverture du fichier échoue
     */
    Csv(const std::string& filename, std::vector<std::string> headers, CsvMode mode = CsvMode::OVERWRITE)
        : filename_(filename) {
        bool file_exists = std::filesystem::exists(filename_);

        if (file_exists && mode == CsvMode::OVERWRITE) {
            std::filesystem::remove(filename_);
            file_exists = false;
        }

        csv_.open(filename_, std::ios::out | std::ios::app);
        if (!csv_.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filename_);
        }

        if (!file_exists) {
            for (size_t i = 0; i < headers.size(); ++i) {
                csv_ << headers[i];
                if (i < headers.size() - 1) {
                    csv_ << ",";
                }
            }
            csv_ << std::endl;
        }
    }

    /**
     * @brief Destructeur qui ferme le fichier CSV
     */
    ~Csv() {
        if (csv_.is_open()) {
            csv_.close();
        }
    }

    /**
     * @brief Ajoute une nouvelle ligne de données au fichier CSV
     *
     * @param data Tuple contenant les valeurs à écrire dans une nouvelle ligne
     */
    void add_row(const std::tuple<Args...>& data) {
        std::apply(
            [this](const auto&... args) {
                size_t i = 0;
                ((csv_ << args << (++i != sizeof...(Args) ? "," : "")), ...);
                csv_ << std::endl;
            },
            data);
    }

  private:
    std::string filename_;
    std::ofstream csv_;
};

#endif