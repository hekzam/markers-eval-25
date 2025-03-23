# Chargement des bibliothèques
library(ggplot2)
library(dplyr)
library(tidyr)
library(corrplot)

# Fonction pour lire et analyser les données
analyser_donnees_code <- function(chemin_fichier) {
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_fichier)
  
  # Affichage des premières lignes pour vérification
  cat("Aperçu des données:\n")
  print(head(donnees))
  
  # Statistiques descriptives
  cat("\nRésumé statistique:\n")
  print(summary(donnees))
  
  # Définir un thème personnalisé pour tous les graphiques
  mon_theme <- theme_minimal() +
    theme(
      plot.background = element_rect(fill = "white", color = "white"),
      panel.background = element_rect(fill = "white", color = "white"),
      text = element_text(color = "darkblue"),
      axis.text = element_text(color = "darkblue"),
      axis.title = element_text(color = "darkblue", face = "bold"),
      plot.title = element_text(color = "darkblue", face = "bold", size = 14),
      plot.subtitle = element_text(color = "darkblue"),
      legend.text = element_text(color = "darkblue"),
      legend.title = element_text(color = "darkblue", face = "bold")
    )
  
  # 1. Corrélation entre les variables
  matrice_cor <- cor(donnees[, 2:5])
  png("correlation_matrix.png", width = 800, height = 800, bg = "white")
  corrplot(matrice_cor, method = "circle", type = "upper", 
           tl.col = "darkblue", tl.srt = 45, col = colorRampPalette(c("blue", "white", "red"))(100),
           title = "Matrice de corrélation des variables",
           bg = "white", addgrid.col = "darkgray")
  dev.off()
  
  # 2. Graphique de dispersion pour les relations entre les variables
  donnees_long <- pivot_longer(donnees, 
                              cols = c(Lignes_de_Code, Temps_Execution_s, 
                                       Memoire_Utilisee_MB, Performance_Score),
                              names_to = "Variable", 
                              values_to = "Valeur")
  
  p1 <- ggplot(donnees, aes(x = Lignes_de_Code, y = Temps_Execution_s)) +
    geom_point(aes(color = Performance_Score), size = 3) +
    geom_smooth(method = "lm", se = TRUE, color = "purple") +
    scale_color_gradient(low = "orangered", high = "royalblue") +
    labs(title = "Relation entre Lignes de Code et Temps d'Exécution",
         subtitle = "Coloré par Score de Performance",
         x = "Nombre de lignes de code",
         y = "Temps d'exécution (s)") +
    mon_theme
  
  # 3. Relation entre mémoire utilisée et performance
  p2 <- ggplot(donnees, aes(x = Memoire_Utilisee_MB, y = Performance_Score)) +
    geom_point(aes(size = Lignes_de_Code), color = "darkgreen", alpha = 0.7) +
    geom_smooth(method = "loess", color = "orangered", se = TRUE) +
    labs(title = "Relation entre Mémoire Utilisée et Score de Performance",
         subtitle = "Taille des points proportionnelle au nombre de lignes de code",
         x = "Mémoire utilisée (MB)",
         y = "Score de performance") +
    mon_theme
  
  # 4. Comparaison de toutes les variables par ID de code
  p3 <- ggplot(donnees_long, aes(x = Code_ID, y = Valeur, group = Variable, color = Variable)) +
    geom_line(size = 1) +
    geom_point(size = 2) +
    scale_color_brewer(palette = "Set1") +
    facet_wrap(~ Variable, scales = "free_y") +
    labs(title = "Comparaison des variables par ID de code",
         x = "ID du code",
         y = "Valeur") +
    mon_theme +
    theme(axis.text.x = element_text(angle = 45, hjust = 1, color = "darkblue"))
  
  # 5. Histogrammes de distribution pour chaque variable
  p4 <- ggplot(donnees_long, aes(x = Valeur, fill = Variable)) +
    geom_histogram(bins = 6, alpha = 0.7) +
    scale_fill_brewer(palette = "Set2") +
    facet_wrap(~ Variable, scales = "free") +
    labs(title = "Distribution des variables",
         x = "Valeur",
         y = "Fréquence") +
    mon_theme +
    theme(legend.position = "bottom")
  
  # Enregistrement des graphiques
  ggsave("relation_lignes_temps.png", p1, width = 10, height = 6, bg = "white")
  ggsave("relation_memoire_performance.png", p2, width = 10, height = 6, bg = "white")
  ggsave("comparaison_variables.png", p3, width = 12, height = 8, bg = "white")
  ggsave("distribution_variables.png", p4, width = 10, height = 6, bg = "white")
  
  # Affichage des graphiques
  print(p1)
  print(p2)
  print(p3)
  print(p4)
  
  # Retourner les données pour d'autres analyses potentielles
  return(donnees)
}

# Exécution de l'analyse
donnees <- analyser_donnees_code("benchmark_performance.csv")

# Analyse supplémentaire: recherche des codes les plus efficaces
cat("\nTop 3 des codes les plus performants:\n")
donnees_triees <- donnees %>% 
  arrange(desc(Performance_Score))
print(head(donnees_triees, 3))

cat("\nRelations identifiées:\n")
# Calculer quelques corrélations importantes
cor_lignes_temps <- cor(donnees$Lignes_de_Code, donnees$Temps_Execution_s)
cor_memoire_perf <- cor(donnees$Memoire_Utilisee_MB, donnees$Performance_Score)

cat("Corrélation entre lignes de code et temps d'exécution:", round(cor_lignes_temps, 2), "\n")
cat("Corrélation entre mémoire utilisée et performance:", round(cor_memoire_perf, 2), "\n")