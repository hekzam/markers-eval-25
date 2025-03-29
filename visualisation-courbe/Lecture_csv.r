# Chargement des bibliothèques
library(ggplot2)
library(dplyr)
library(tidyr)
library(corrplot)

# Fonction pour lire et analyser les données
analyser_donnees_copies <- function(chemin_fichier) {
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_fichier)
  
  # Ajout d'un numéro de copie extrait du nom de fichier
  donnees$Numero_Copie <- as.numeric(gsub("copy([0-9]+)\\.png", "\\1", donnees$File))
  
  # Convertir le temps en millisecondes en secondes pour une meilleure lisibilité
  donnees$Temps_sec <- donnees$Time.ms. / 1000
  
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
  
  # 1. Graphique linéaire du temps d'exécution par numéro de copie
  p1 <- ggplot(donnees, aes(x = Numero_Copie, y = Temps_sec)) +
    geom_line(color = "royalblue", size = 1) +
    geom_point(color = "darkblue", size = 3) +
    labs(title = "Temps d'exécution pour chaque copie",
         subtitle = "Évolution du temps de traitement",
         x = "Numéro de copie",
         y = "Temps d'exécution (secondes)") +
    scale_x_continuous(breaks = donnees$Numero_Copie) +
    mon_theme
  
  # 2. Comparaison avec la moyenne (ligne de référence)
  temps_moyen <- mean(donnees$Temps_sec)
  ecart_type <- sd(donnees$Temps_sec)
  
  p2 <- ggplot(donnees, aes(x = Numero_Copie, y = Temps_sec)) +
    geom_hline(yintercept = temps_moyen, linetype = "dashed", color = "orangered", size = 1) +
    geom_ribbon(aes(ymin = temps_moyen - ecart_type, ymax = temps_moyen + ecart_type), 
                fill = "lightblue", alpha = 0.3) +
    geom_line(color = "royalblue", size = 1) +
    geom_point(size = 3, aes(color = Temps_sec > temps_moyen)) +
    scale_color_manual(values = c("darkgreen", "red"), 
                       labels = c("Inférieur à la moyenne", "Supérieur à la moyenne"),
                       name = "Performance") +
    labs(title = "Comparaison des temps par rapport à la moyenne",
         subtitle = paste("Temps moyen:", round(temps_moyen, 5), "sec. Écart-type:", round(ecart_type, 5), "sec"),
         x = "Numéro de copie",
         y = "Temps d'exécution (secondes)") +
    scale_x_continuous(breaks = donnees$Numero_Copie) +
    mon_theme
  
  # 3. Histogramme de distribution des temps
  p3 <- ggplot(donnees, aes(x = Temps_sec)) +
    geom_histogram(bins = 8, fill = "steelblue", color = "darkblue", alpha = 0.7) +
    geom_vline(xintercept = temps_moyen, linetype = "dashed", color = "red", size = 1) +
    labs(title = "Distribution des temps d'exécution",
         subtitle = "Fréquence des différents temps de traitement",
         x = "Temps d'exécution (secondes)",
         y = "Nombre de copies") +
    mon_theme
  
  # 4. Temps d'exécution par groupe de copies (par tranches de 5)
  donnees$Groupe_Copie <- cut(donnees$Numero_Copie, 
                             breaks = c(0, 5, 10, 15), 
                             labels = c("Copies 1-5", "Copies 6-10", "Copies 11-15"))
  
  p4 <- ggplot(donnees, aes(x = Groupe_Copie, y = Temps_sec, fill = Groupe_Copie)) +
    geom_boxplot(alpha = 0.7) +
    geom_jitter(width = 0.2, height = 0, size = 2, alpha = 0.7) +
    labs(title = "Comparaison des temps d'exécution par groupe de copies",
         subtitle = "Analyse par tranches de 5 copies",
         x = "Groupe de copies",
         y = "Temps d'exécution (secondes)") +
    scale_fill_brewer(palette = "Blues") +
    mon_theme +
    theme(legend.position = "none")
  
  # 5. Courbe de tendance avec régression
  p5 <- ggplot(donnees, aes(x = Numero_Copie, y = Temps_sec)) +
    geom_point(aes(size = Temps_sec), color = "darkblue", alpha = 0.7) +
    geom_smooth(method = "loess", color = "red", se = TRUE) +
    labs(title = "Tendance d'évolution du temps d'exécution",
         subtitle = "Avec courbe de régression locale (LOESS)",
         x = "Numéro de copie",
         y = "Temps d'exécution (secondes)") +
    scale_x_continuous(breaks = donnees$Numero_Copie) +
    mon_theme
  
  # 6. Graphique de déviation par rapport à la moyenne
  donnees$Deviation <- donnees$Temps_sec - temps_moyen
  donnees$Direction <- ifelse(donnees$Deviation >= 0, "Positif", "Négatif")
  
  p6 <- ggplot(donnees, aes(x = Numero_Copie, y = Deviation, fill = Direction)) +
    geom_col(width = 0.7) +
    geom_hline(yintercept = 0, linetype = "solid", color = "black", size = 1) +
    scale_fill_manual(values = c("Négatif" = "forestgreen", "Positif" = "firebrick")) +
    labs(title = "Déviation du temps d'exécution par rapport à la moyenne",
         subtitle = "Valeurs positives = plus lent que la moyenne, négatives = plus rapide",
         x = "Numéro de copie",
         y = "Déviation (secondes)") +
    scale_x_continuous(breaks = donnees$Numero_Copie) +
    mon_theme
  
  # Enregistrement des graphiques
  ggsave("temps_execution_copies.png", p1, width = 10, height = 6, bg = "white")
  ggsave("comparaison_moyenne.png", p2, width = 10, height = 6, bg = "white")
  ggsave("distribution_temps.png", p3, width = 8, height = 6, bg = "white")
  ggsave("comparison_groupes.png", p4, width = 8, height = 6, bg = "white")
  ggsave("tendance_evolution.png", p5, width = 10, height = 6, bg = "white")
  ggsave("deviation_moyenne.png", p6, width = 10, height = 6, bg = "white")
  
  # Affichage des graphiques
  print(p1)
  print(p2)
  print(p3)
  print(p4)
  print(p5)
  print(p6)
  
  # Retourner les données pour d'autres analyses potentielles
  return(donnees)
}

# Définir le répertoire de travail sur le dossier contenant le CSV
setwd("\\markers-eval-25\\visualisation-courbe")
# Exécution de l'analyse avec le chemin du fichier CSV (nom correctement orthographié)
donnees <- analyser_donnees_copies("benchmark_performance.csv")

# Analyse supplémentaire: identification deas copies les plus rapides et les plus lentes
cat("\nCopies triées par temps d'exécution (de la plus rapide à la plus lente):\n")
donnees_triees <- donnees %>% 
  arrange(Temps_sec)
print(donnees_triees)

cat("\nStatistiques complémentaires:\n")
cat("Temps d'exécution moyen:", mean(donnees$Temps_sec), "secondes\n")
cat("Temps d'exécution médian:", median(donnees$Temps_sec), "secondes\n")
cat("Écart-type des temps d'exécution:", sd(donnees$Temps_sec), "secondes\n")
cat("Copie la plus rapide:", donnees$File[which.min(donnees$Temps_sec)], "avec", min(donnees$Temps_sec), "secondes\n")
cat("Copie la plus lente:", donnees$File[which.max(donnees$Temps_sec)], "avec", max(donnees$Temps_sec), "secondes\n")