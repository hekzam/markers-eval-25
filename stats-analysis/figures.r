# Chargement des bibliothèques
library(ggplot2)
library(dplyr)
library(tidyr)
library(corrplot)

# Fonction pour lire et analyser les données
analyser_donnees_copies <- function(chemin_fichier) {
  # Vérifier si le fichier existe
  if (!file.exists(chemin_fichier)) {
    stop("Erreur : Le fichier CSV n'existe pas dans le répertoire indiqué.")
  }
  
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_fichier, stringsAsFactors = FALSE)
  
  # Vérifier si les colonnes nécessaires existent
  if (!all(c("File", "Time.ms.") %in% colnames(donnees))) {
    stop("Erreur : Le fichier CSV ne contient pas les colonnes attendues (File, Time.ms.).")
  }
  
  # Extraire le numéro de copie à partir du nom du fichier
  donnees$Numero_Copie <- as.numeric(gsub("copy([0-9]+)\\.png", "\\1", donnees$File))
  
  # Vérifier si l'extraction a réussi
  if (any(is.na(donnees$Numero_Copie))) {
    stop("Erreur : Impossible d'extraire les numéros de copie. Vérifiez les noms de fichiers.")
  }
  
  # Convertir le temps en millisecondes (sans division)
  donnees$Temps_ms <- donnees$Time.ms.
  
  # Affichage des premières lignes pour vérification
  cat("Aperçu des données :\n")
  print(head(donnees))
  
  # Statistiques descriptives
  cat("\nRésumé statistique :\n")
  print(summary(donnees))
  
  # Définir un thème personnalisé pour tous les graphiques
  mon_theme <- theme_minimal() +
    theme(
      text = element_text(color = "darkblue"),
      axis.text = element_text(color = "darkblue"),
      axis.title = element_text(color = "darkblue", face = "bold"),
      plot.title = element_text(color = "darkblue", face = "bold", size = 14),
      plot.subtitle = element_text(color = "darkblue"),
      legend.text = element_text(color = "darkblue"),
      legend.title = element_text(color = "darkblue", face = "bold")
    )
  
  # Calculer la moyenne et l'écart-type
  temps_moyen <- mean(donnees$Temps_ms, na.rm = TRUE)
  ecart_type <- sd(donnees$Temps_ms, na.rm = TRUE)
  
  # 1. Graphique linéaire du temps d'exécution par numéro de copie
  p1 <- ggplot(donnees, aes(x = Numero_Copie, y = Temps_ms)) +
    geom_line(color = "royalblue", size = 1) +
    geom_point(color = "darkblue", size = 3) +
    labs(title = "Temps d'exécution pour chaque copie",
         subtitle = "Évolution du temps de traitement",
         x = "Numéro de copie",
         y = "Temps d'exécution (ms)") +
    mon_theme
  
  # 2. Comparaison avec la moyenne
  p2 <- ggplot(donnees, aes(x = Numero_Copie, y = Temps_ms)) +
    geom_hline(yintercept = temps_moyen, linetype = "dashed", color = "orangered", size = 1) +
    geom_ribbon(aes(ymin = temps_moyen - ecart_type, ymax = temps_moyen + ecart_type), 
                fill = "lightblue", alpha = 0.3) +
    geom_line(color = "royalblue", size = 1) +
    geom_point(size = 3, aes(color = Temps_ms > temps_moyen)) +
    scale_color_manual(values = c("darkgreen", "red"),
                       labels = c("Inférieur à la moyenne", "Supérieur à la moyenne"),
                       name = "Performance") +
    labs(title = "Comparaison des temps par rapport à la moyenne",
         subtitle = paste("Temps moyen:", round(temps_moyen, 5), "ms. Écart-type:", round(ecart_type, 5), "ms"),
         x = "Numéro de copie",
         y = "Temps d'exécution (ms)") +
    mon_theme
  
  # 3. Histogramme de distribution des temps
  p3 <- ggplot(donnees, aes(x = Temps_ms)) +
    geom_histogram(bins = 10, fill = "steelblue", color = "darkblue", alpha = 0.7) +
    geom_vline(xintercept = temps_moyen, linetype = "dashed", color = "red", size = 1) +
    labs(title = "Distribution des temps d'exécution",
         subtitle = "Fréquence des différents temps de traitement",
         x = "Temps d'exécution (ms)",
         y = "Nombre de copies") +
    mon_theme
  
  # 4. Temps d'exécution par groupe de copies (groupé dynamiquement)
  nb_groupes <- ceiling(max(donnees$Numero_Copie) / 5)
  donnees$Groupe_Copie <- cut(donnees$Numero_Copie, 
                              breaks = seq(0, max(donnees$Numero_Copie) + 5, by = 5), 
                              include.lowest = TRUE, right = FALSE)
  
  p4 <- ggplot(donnees, aes(x = Groupe_Copie, y = Temps_ms, fill = Groupe_Copie)) +
    geom_boxplot(alpha = 0.7) +
    geom_jitter(width = 0.2, height = 0, size = 2, alpha = 0.7) +
    labs(title = "Comparaison des temps d'exécution par groupe de copies",
         subtitle = "Analyse par tranches de 5 copies",
         x = "Groupe de copies",
         y = "Temps d'exécution (ms)") +
    scale_fill_brewer(palette = "Blues") +
    mon_theme +
    theme(legend.position = "none")
  
  # Enregistrer et afficher les graphiques
  ggsave("temps_execution_copies.png", p1, width = 10, height = 6, bg = "white")
  ggsave("comparaison_moyenne.png", p2, width = 10, height = 6, bg = "white")
  ggsave("distribution_temps.png", p3, width = 8, height = 6, bg = "white")
  ggsave("comparison_groupes.png", p4, width = 8, height = 6, bg = "white")
  
  print(p1)
  print(p2)
  print(p3)
  print(p4)
  
  # Retourner les données pour d'autres analyses potentielles
  return(donnees)
}

# Vérifier et définir le répertoire de travail
if (dir.exists("markers-eval-25/stats-analysis")) {
  setwd("markers-eval-25/stats-analysis")
} else {
  cat("Attention : Le répertoire de travail n'existe pas.\n")
}

# Exécution de l'analyse avec le chemin du fichier CSV
donnees <- analyser_donnees_copies("benchmark_performance.csv")

# Analyse des performances
cat("\nCopies triées par temps d'exécution :\n")
donnees_triees <- donnees %>% arrange(Temps_ms)
print(donnees_triees)

# Statistiques complémentaires
cat("\nStatistiques complémentaires :\n")
cat("Temps moyen :", mean(donnees$Temps_ms), "ms\n")
cat("Temps médian :", median(donnees$Temps_ms), "ms\n")
cat("Écart-type :", sd(donnees$Temps_ms), "ms\n")
cat("Copie la plus rapide :", donnees$File[which.min(donnees$Temps_ms)], "\n")
cat("Copie la plus lente :", donnees$File[which.max(donnees$Temps_ms)], "\n")
