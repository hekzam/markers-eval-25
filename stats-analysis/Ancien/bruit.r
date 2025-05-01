library(ggplot2)
library(dplyr)

analyser_impact_bruit <- function(chemin_csv) {
  if (!file.exists(chemin_csv)) stop("Fichier introuvable.")
  
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_csv, stringsAsFactors = FALSE)
  
  # Renommer la colonne Time(ms) pour qu'elle soit plus facile à manipuler
  colnames(donnees)[colnames(donnees) == "Time(ms)"] <- "Time_ms"
  
  
  # Statistiques par niveau de bruit
  stats_bruit <- donnees %>%
    group_by(Noise_Level) %>%
    summarise(
      Moyenne = mean(Time_ms),
      IC_bas = Moyenne - qt(0.975, df = n() - 1) * sd(Time_ms) / sqrt(n()),
      IC_haut = Moyenne + qt(0.975, df = n() - 1) * sd(Time_ms) / sqrt(n()),
      .groups = "drop"
    )
  
  # Graphique avec points par copie et courbe moyenne + IC
  p <- ggplot() +
    # Points individuels par copie
    geom_point(data = donnees, aes(x = Noise_Level, y = Time_ms), 
           color = "darkred", alpha = 0.6, size = 3, shape = 16) +

    
    # Ligne moyenne
    # geom_line(data = stats_bruit, aes(x = Noise_Level, y = Moyenne), 
    #           color = "darkblue", linewidth = 1.2) +
    
    # Points moyens
    geom_point(data = stats_bruit, aes(x = Noise_Level, y = Moyenne), 
               size = 4, color = "steelblue") +
    
    # Intervalle de confiance
    geom_ribbon(data = stats_bruit, 
                aes(x = Noise_Level, ymin = IC_bas, ymax = IC_haut), 
                fill = "lightblue", alpha = 0.3) +
    
    # Titre et axes
    labs(
      title = "Impact du bruit sur le temps de parsing",
      subtitle = "Chaque point correspond à une copie individuelle. Moyenne + IC à 95%",
      x = "Niveau de bruit (%)",
      y = "Temps de parsing (ms)"
    ) +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue")
    )
  
  if (interactive()) {
  print(p)
}

  ggsave("impact_bruit_temps_detaille.png", p, width = 10, height = 7, bg = "white")
  
  return(list(stats = stats_bruit, donnees = donnees))
}

# Exécution
args <- commandArgs(trailingOnly = TRUE)
resultats <- analyser_impact_bruit(args[1])
