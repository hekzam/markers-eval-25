####################################################
# SCRIPT RESTRUCTURÉ D'ANALYSE DE DONNÉES PAR PARSEUR
# Version améliorée du script fusionné
####################################################

# Chargement des bibliothèques
library(ggplot2)
library(dplyr)
library(tidyr)
library(fs) # Pour la gestion des dossiers

# Liste des parseurs à analyser
PARSEURS <- c("aruco_parser", "circle_parser", "custom_marker_parser", 
              "default_parser", "qrcode_empty_parser", "qrcode_parser", "shape_parser")

# Fonction pour créer des dossiers s'ils n'existent pas
creer_dossiers <- function() {
  # Créer un dossier principal pour les résultats
  dir_create("analysis_results", recurse = TRUE)
  
  # Créer un sous-dossier pour chaque parseur
  for (parseur in PARSEURS) {
    dir_create(paste0("analysis_results/", parseur), recurse = TRUE)
  }
  
  cat("Dossiers créés avec succès\n")
}

# Fonction pour filtrer les données par parseur
filtrer_par_parseur <- function(donnees, nom_parseur) {
  # Vérifier si la colonne Parser existe
  if (!("Parser" %in% colnames(donnees))) {
    stop("Erreur : Le fichier CSV ne contient pas la colonne 'Parser'.")
  }
  
  # Filtrer les données pour le parseur spécifié
  donnees_filtrees <- donnees %>%
    filter(Parser == nom_parseur)
  
  # Vérifier si des données ont été trouvées pour ce parseur
  if (nrow(donnees_filtrees) == 0) {
    warning(paste("Aucune donnée trouvée pour le parseur", nom_parseur))
    return(NULL)
  }
  
  return(donnees_filtrees)
}

# Fonction pour analyser les données d'un parseur spécifique
analyser_donnees_parseur <- function(donnees, nom_parseur) {
  cat(paste("\nAnalyse du parseur:", nom_parseur, "\n"))
  
  # Créer le chemin du dossier pour ce parseur
  dossier_parseur <- paste0("analysis_results/", nom_parseur)
  
  # Extraire le numéro de copie original à partir du nom du fichier
  donnees$Numero_Copie_Original <- as.numeric(gsub("copy([0-9]+)\\.png", "\\1", donnees$File))
  
  # Vérifier si l'extraction a réussi
  if (any(is.na(donnees$Numero_Copie_Original))) {
    warning("Attention : Impossible d'extraire certains numéros de copie. Vérifiez les noms de fichiers.")
  }
  
  # Réindexer pour obtenir une séquence consécutive pour ce parseur spécifique
  # Trier d'abord par numéro de copie original
  donnees <- donnees[order(donnees$Numero_Copie_Original), ]
  # Créer un nouveau numéro de séquence consécutif (1, 2, 3, ...)
  donnees$Numero_Copie <- seq_len(nrow(donnees))
  
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
  temps_moyen <- mean(donnees$Time_ms, na.rm = TRUE)
  ecart_type <- sd(donnees$Time_ms, na.rm = TRUE)
  
  # 1. Graphique linéaire du temps d'exécution par numéro de copie
  p1 <- ggplot(donnees, aes(x = Numero_Copie, y = Time_ms)) +
    geom_point(color = "darkblue", size = 3) +
    labs(title = paste("Temps d'exécution pour chaque copie -", nom_parseur),
         subtitle = "Évolution du temps de traitement",
         x = "Numéro de copie",
         y = "Temps d'exécution (ms)") +
    mon_theme
  
  # 2. Comparaison avec la moyenne
  p2 <- ggplot(donnees, aes(x = Numero_Copie, y = Time_ms)) +
    geom_hline(yintercept = temps_moyen, linetype = "dashed", color = "orangered", size = 1) +
    geom_ribbon(aes(ymin = temps_moyen - ecart_type, ymax = temps_moyen + ecart_type), 
                fill = "lightblue", alpha = 0.3) +
    geom_point(size = 3, aes(color = Time_ms > temps_moyen)) +
    scale_color_manual(values = c("darkgreen", "red"),
                       labels = c("Inférieur à la moyenne", "Supérieur à la moyenne"),
                       name = "Performance") +
    labs(title = paste("Comparaison des temps par rapport à la moyenne -", nom_parseur),
         subtitle = paste("Temps moyen:", round(temps_moyen, 5), "ms. Écart-type:", round(ecart_type, 5), "ms"),
         x = "Numéro de copie",
         y = "Temps d'exécution (ms)") +
    mon_theme
  
  # 3. Histogramme de distribution des temps
  p3 <- ggplot(donnees, aes(x = Time_ms)) +
    geom_histogram(bins = 10, fill = "steelblue", color = "darkblue", alpha = 0.7) +
    geom_vline(xintercept = temps_moyen, linetype = "dashed", color = "red", size = 1) +
    labs(title = paste("Distribution des temps d'exécution -", nom_parseur),
         subtitle = "Fréquence des différents temps de traitement",
         x = "Temps d'exécution (ms)",
         y = "Nombre de copies") +
    mon_theme
  
  # 4. Temps d'exécution par groupe de copies (groupé dynamiquement)
  nb_groupes <- ceiling(max(donnees$Numero_Copie, na.rm = TRUE) / 5)
  donnees$Groupe_Copie <- cut(donnees$Numero_Copie, 
                              breaks = seq(0, max(donnees$Numero_Copie, na.rm = TRUE) + 5, by = 5), 
                              include.lowest = TRUE, right = FALSE)
  
  p4 <- ggplot(donnees, aes(x = Groupe_Copie, y = Time_ms, fill = Groupe_Copie)) +
    geom_boxplot(alpha = 0.7) +
    geom_jitter(width = 0.2, height = 0, size = 2, alpha = 0.7) +
    labs(title = paste("Comparaison des temps d'exécution par groupe de copies -", nom_parseur),
         subtitle = "Analyse par tranches de 5 copies",
         x = "Groupe de copies",
         y = "Temps d'exécution (ms)") +
    scale_fill_brewer(palette = "Blues") +
    mon_theme +
    theme(legend.position = "none")
  
  # Enregistrer les graphiques dans le dossier du parseur
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_temps_execution_copies.png"), p1, width = 10, height = 6, bg = "white")
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_comparaison_moyenne.png"), p2, width = 10, height = 6, bg = "white")
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_distribution_temps.png"), p3, width = 8, height = 6, bg = "white")
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_comparison_groupes.png"), p4, width = 8, height = 6, bg = "white")
  
  if (interactive()) {
    print(p1)
    print(p2)
    print(p3)
    print(p4)
  }
  
  # Statistiques complémentaires
  cat("\nStatistiques complémentaires pour le parseur", nom_parseur, ":\n")
  cat("Temps moyen :", mean(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian :", median(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type :", sd(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Copie la plus rapide :", donnees$File[which.min(donnees$Time_ms)], "\n")
  cat("Copie la plus lente :", donnees$File[which.max(donnees$Time_ms)], "\n")
  
  # Enregistrer les statistiques dans un fichier texte
  stats_file <- paste0(dossier_parseur, "/", nom_parseur, "_statistiques.txt")
  sink(stats_file)
  cat("Statistiques pour le parseur", nom_parseur, ":\n\n")
  cat("Nombre de copies analysées :", nrow(donnees), "\n")
  cat("Temps moyen :", mean(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian :", median(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type :", sd(donnees$Time_ms, na.rm = TRUE), "ms\n")
  cat("Copie la plus rapide :", donnees$File[which.min(donnees$Time_ms)], "\n")
  cat("Copie la plus lente :", donnees$File[which.max(donnees$Time_ms)], "\n")
  sink()
  
  # Retourner les données pour d'autres analyses potentielles
  return(donnees)
}

# Fonction pour analyser l'impact du bruit par parseur
analyser_impact_bruit_parseur <- function(donnees, nom_parseur) {
  # Vérifier si la colonne Noise_Level existe
  if (!("Noise_Level" %in% colnames(donnees))) {
    cat("Avertissement : Le fichier CSV ne contient pas la colonne 'Noise_Level'. Analyse du bruit ignorée.\n")
    return(NULL)
  }
  
  # Créer le chemin du dossier pour ce parseur
  dossier_parseur <- paste0("analysis_results/", nom_parseur)
  
  # Statistiques par niveau de bruit
  stats_bruit <- donnees %>%
    group_by(Noise_Level) %>%
    summarise(
      Moyenne = mean(Time_ms, na.rm = TRUE),
      IC_bas = Moyenne - qt(0.975, df = n() - 1) * sd(Time_ms, na.rm = TRUE) / sqrt(n()),
      IC_haut = Moyenne + qt(0.975, df = n() - 1) * sd(Time_ms, na.rm = TRUE) / sqrt(n()),
      .groups = "drop"
    )
  
  # Graphique avec points par copie et courbe moyenne + IC
  p <- ggplot() +
    # Points individuels par copie
    geom_point(data = donnees, aes(x = Noise_Level, y = Time_ms), 
               color = "darkred", alpha = 0.6, size = 3, shape = 16) +
    
    # Points moyens
    geom_point(data = stats_bruit, aes(x = Noise_Level, y = Moyenne), 
               size = 4, color = "steelblue") +
    
    # Intervalle de confiance
    geom_ribbon(data = stats_bruit, 
                aes(x = Noise_Level, ymin = IC_bas, ymax = IC_haut), 
                fill = "lightblue", alpha = 0.3) +
    
    # Titre et axes
    labs(
      title = paste("Impact du bruit sur le temps de parsing -", nom_parseur),
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
  
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_impact_bruit_temps.png"), p, width = 10, height = 7, bg = "white")
  
  return(stats_bruit)
}

# Fonction pour analyser les différences entre les parseurs
analyser_differences_parseurs <- function(donnees_completes) {
  # Créer le dossier pour l'analyse comparative
  dir_create("analysis_results/comparaison_parseurs", recurse = TRUE)
  
  # Calculer les statistiques par parseur
  stats_parseurs <- donnees_completes %>%
    group_by(Parser) %>%
    summarise(
      Nb_Copies = n(),
      Temps_Moyen = mean(Time_ms, na.rm = TRUE),
      Temps_Median = median(Time_ms, na.rm = TRUE),
      Ecart_Type = sd(Time_ms, na.rm = TRUE),
      Temps_Min = min(Time_ms, na.rm = TRUE),
      Temps_Max = max(Time_ms, na.rm = TRUE),
      .groups = "drop"
    ) %>%
    arrange(Temps_Moyen)
  
  # Écrire les statistiques dans un fichier
  write.csv(stats_parseurs, "analysis_results/comparaison_parseurs/statistiques_parseurs.csv", row.names = FALSE)
  
  # Créer un graphique comparatif des temps moyens
  p1 <- ggplot(stats_parseurs, aes(x = reorder(Parser, Temps_Moyen), y = Temps_Moyen)) +
    geom_bar(stat = "identity", fill = "steelblue", alpha = 0.8) +
    geom_errorbar(aes(ymin = Temps_Moyen - Ecart_Type, ymax = Temps_Moyen + Ecart_Type), width = 0.2) +
    labs(title = "Comparaison des temps moyens par parseur",
         subtitle = "Avec écart-type",
         x = "Parseur",
         y = "Temps moyen (ms)") +
    theme_minimal() +
    theme(
      axis.text.x = element_text(angle = 45, hjust = 1),
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue")
    )
  
  ggsave("analysis_results/comparaison_parseurs/comparaison_temps_moyens.png", p1, width = 12, height = 8, bg = "white")
  
  # Créer un boxplot pour comparer les distributions
  p2 <- ggplot(donnees_completes, aes(x = reorder(Parser, Time_ms, FUN = median), y = Time_ms)) +
    geom_boxplot(aes(fill = Parser), alpha = 0.7) +
    labs(title = "Distribution des temps d'exécution par parseur",
         x = "Parseur",
         y = "Temps d'exécution (ms)") +
    theme_minimal() +
    theme(
      axis.text.x = element_text(angle = 45, hjust = 1),
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none"
    )
  
  ggsave("analysis_results/comparaison_parseurs/distribution_temps_parseurs.png", p2, width = 12, height = 8, bg = "white")
  
  # Si la colonne Noise_Level existe, créer un graphique d'interaction
  if ("Noise_Level" %in% colnames(donnees_completes)) {
    # Calculer les moyennes par parseur et niveau de bruit
    stats_bruit_parseurs <- donnees_completes %>%
      group_by(Parser, Noise_Level) %>%
      summarise(
        Temps_Moyen = mean(Time_ms, na.rm = TRUE),
        .groups = "drop"
      )
    
    p3 <- ggplot(stats_bruit_parseurs, aes(x = Noise_Level, y = Temps_Moyen, color = Parser, group = Parser)) +
      geom_line(linewidth = 1.2) +
      geom_point(size = 3) +
      labs(title = "Évolution du temps d'exécution selon le niveau de bruit",
           subtitle = "Comparaison entre les parseurs",
           x = "Niveau de bruit (%)",
           y = "Temps moyen (ms)") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue")
      )
    
    ggsave("analysis_results/comparaison_parseurs/effet_bruit_parseurs.png", p3, width = 12, height = 8, bg = "white")
  }
  
  return(stats_parseurs)
}

# Fonction principale pour analyser tous les parseurs
analyser_tous_parseurs <- function(chemin_csv) {
  # Vérifier si le fichier existe
  if (!file.exists(chemin_csv)) {
    stop("Erreur : Le fichier CSV n'existe pas dans le répertoire indiqué.")
  }
  
  # Lecture du fichier CSV
  donnees_completes <- read.csv(chemin_csv, stringsAsFactors = FALSE)
  
  # Vérifier si les colonnes nécessaires existent
  if (!all(c("File", "Time_ms", "Parser") %in% colnames(donnees_completes))) {
    stop("Erreur : Le fichier CSV ne contient pas les colonnes attendues (File, Time_ms, Parser).")
  }
  
  # Créer les dossiers nécessaires
  creer_dossiers()
  
  cat("\n========== ANALYSE PAR PARSEUR ==========\n\n")
  
  # Stocker les résultats pour chaque parseur
  resultats_parseurs <- list()
  
  # Pour chaque parseur, analyser ses données
  for (parseur in PARSEURS) {
    # Filtrer les données pour ce parseur
    donnees_parseur <- filtrer_par_parseur(donnees_completes, parseur)
    
    if (!is.null(donnees_parseur)) {
      # Analyser les performances générales
      donnees_analysees <- analyser_donnees_parseur(donnees_parseur, parseur)
      
      # Analyser l'impact du bruit si applicable
      stats_bruit <- analyser_impact_bruit_parseur(donnees_parseur, parseur)
      
      # Stocker les résultats
      resultats_parseurs[[parseur]] <- list(
        donnees = donnees_analysees,
        stats_bruit = stats_bruit
      )
    }
  }
  
  # Analyser les différences entre les parseurs
  cat("\n----- ANALYSE COMPARATIVE DES PARSEURS -----\n")
  stats_parseurs <- analyser_differences_parseurs(donnees_completes)
  
  cat("\n========== ANALYSE TERMINÉE ==========\n")
  cat("Les graphiques ont été enregistrés dans le dossier 'analysis_results'.\n")
  
  return(list(resultats_parseurs = resultats_parseurs, stats_parseurs = stats_parseurs))
}

# Point d'entrée du script
main <- function() {
  args <- commandArgs(trailingOnly = TRUE)
  
  if (length(args) == 0) {
    cat("Aucun fichier CSV spécifié. Utilisation du fichier par défaut 'csv_parfait.csv'.\n")
    chemin_csv <- "csv_parfait.csv"
  } else {
    chemin_csv <- args[1]
  }
  
  resultats <- analyser_tous_parseurs(chemin_csv)
  return(resultats)
}

# Exécution du script si lancé directement
if (!interactive()) {
  main()
}