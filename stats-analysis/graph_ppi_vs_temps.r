# Chargement des bibliothèques
library(ggplot2)
library(dplyr)

# Chemin vers le fichier CSV
chemin_csv <- "benchmark_bruit.csv"

# Vérification et lecture
if (!file.exists(chemin_csv)) stop("Fichier introuvable.")
donnees <- read.csv(chemin_csv, stringsAsFactors = FALSE)

# Renommer la colonne Time(ms) si nécessaire
if ("Time(ms)" %in% colnames(donnees)) {
  colnames(donnees)[colnames(donnees) == "Time(ms)"] <- "Time_ms"
}

# Vérifier la présence de PPI et Noise_Level
if (!("PPI" %in% colnames(donnees)) || !("Noise_Level" %in% colnames(donnees))) {
  stop("Colonnes PPI ou Noise_Level manquantes dans le fichier.")
}

# Création du graphique
p <- ggplot(donnees, aes(x = PPI, y = Time_ms, color = as.factor(Noise_Level))) +
  geom_point(size = 3, alpha = 0.7) +
  labs(
    title = "Temps de parsing en fonction du PPI",
    subtitle = "Chaque point correspond à une copie, colorée par niveau de bruit",
    x = "PPI (résolution)",
    y = "Temps de parsing (ms)",
    color = "Niveau de bruit (%)"
  ) +
  theme_minimal() +
  theme(
    plot.title = element_text(face = "bold", size = 14, color = "darkred"),
    axis.title = element_text(face = "bold", color = "firebrick")
  )

# Sauvegarde en PNG
ggsave("temps_vs_ppi.png", plot = p, width = 10, height = 7, dpi = 300, bg = "white")
