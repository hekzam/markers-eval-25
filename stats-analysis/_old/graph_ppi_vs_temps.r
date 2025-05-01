# Chargement des bibliothèques
library(ggplot2)
library(dplyr)

# Chemin vers le fichier CSV
args <- commandArgs(trailingOnly = TRUE)
chemin_csv <- args[1]


# Vérification et lecture
if (!file.exists(chemin_csv)) stop("Fichier introuvable.")
donnees <- read.csv(chemin_csv, stringsAsFactors = FALSE)

# Renommer la colonne Time(ms) si nécessaire
if ("Time(ms)" %in% colnames(donnees)) {
  colnames(donnees)[colnames(donnees) == "Time(ms)"] <- "Time_ms"
}

# Vérification des colonnes nécessaires
if (!("PPI" %in% colnames(donnees)) || !("Parser" %in% colnames(donnees))) {
  stop("Colonnes PPI ou Parser manquantes dans le fichier.")
}

# Création du graphique
p <- ggplot(donnees, aes(x = PPI, y = Time_ms, color = as.factor(Parser))) +
  geom_point(size = 3, alpha = 0.7) +
  labs(
    title = "Temps de parsing en fonction du PPI",
    subtitle = "Chaque point correspond à une copie, colorée par type de parser",
    x = "PPI (résolution)",
    y = "Temps de parsing (ms)",
    color = "Parser"
  ) +
  theme_minimal() +
  theme(
    plot.title = element_text(face = "bold", size = 14, color = "darkred"),
    axis.title = element_text(face = "bold", color = "firebrick")
  )

# Sauvegarde en PNG
ggsave("temps_vs_ppi_par_parser.png", plot = p, width = 10, height = 7, dpi = 300, bg = "white")
