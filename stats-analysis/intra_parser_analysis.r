####################################################
# SCRIPT RESTRUCTURÉ D'ANALYSE DE DONNÉES PAR PARSEUR
# Version adaptée pour nouvelles colonnes CSV
####################################################

# Chargement des bibliothèques
library(ggplot2)
library(dplyr)
library(tidyr)
library(fs) # Pour la gestion des dossiers

# Liste des parseurs à analyser
PARSEURS <- c("ZXING", "CIRCLE", "SHAPE", 
              "ARUCO", "CENTER_PARSER")

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
  # Vérifier si la colonne Parser_Type existe
  if (!("Parser_Type" %in% colnames(donnees))) {
    stop("Erreur : Le fichier CSV ne contient pas la colonne 'Parser_Type'.")
  }
  
  # Filtrer les données pour le parseur spécifié
  donnees_filtrees <- donnees %>%
    filter(Parser_Type == nom_parseur)
  
  # Vérifier si des données ont été trouvées pour ce parseur
  if (nrow(donnees_filtrees) == 0) {
    warning(paste("Aucune donnée trouvée pour le parseur", nom_parseur))
    return(NULL)
  }
  
  return(donnees_filtrees)
}

# Nouvelle fonction pour générer le graphique barres_precision_par_config pour un parseur
generer_barres_precision_par_config <- function(donnees, nom_parseur) {
  # Vérifier la présence des colonnes nécessaires
  if (!("Copy_Config" %in% colnames(donnees)) || !("Precision_Error_Avg_px" %in% colnames(donnees))) {
    cat("Graphique barres_precision_par_config non généré : colonnes manquantes pour", nom_parseur, "\n")
    return(NULL)
  }
  # Filtrer les valeurs aberrantes (-1)
  donnees_precision <- donnees %>% filter(Precision_Error_Avg_px != -1)
  # Calculer les statistiques par configuration
  stats_config <- donnees_precision %>%
    group_by(Copy_Config) %>%
    summarise(
      Erreur_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
      Erreur_Ecart_Type = sd(Precision_Error_Avg_px, na.rm = TRUE),
      .groups = "drop"
    )
  # Créer le graphique
  p <- ggplot(stats_config, aes(x = Copy_Config, y = Erreur_Moyenne, fill = Copy_Config)) +
    geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
    geom_errorbar(aes(
      ymin = pmax(Erreur_Moyenne - Erreur_Ecart_Type, 0),
      ymax = Erreur_Moyenne + Erreur_Ecart_Type
    ),
    width = 0.2) +
    scale_fill_brewer(palette = "Set2") +
    labs(title = paste0("Erreur moyenne de précision par configuration - ", nom_parseur),
         x = "Configuration",
         y = "Erreur moyenne (pixels)",
         fill = "Configuration") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(angle = 45, hjust = 1)
    ) +
    scale_y_continuous(limits = c(0, NA))
  # Sauvegarder dans le dossier du parseur
  dossier_parseur <- paste0("analysis_results/", nom_parseur)
  ggsave(paste0(dossier_parseur, "/barres_precision_par_config_", nom_parseur, ".png"), p, width = 12, height = 7, bg = "white")
  if (interactive()) print(p)
  return(p)
}

# Nouvelle fonction pour générer les scatter plots temps vs précision pour un parseur
generer_scatter_temps_vs_precision <- function(donnees, nom_parseur) {
  if (!("Precision_Error_Avg_px" %in% colnames(donnees)) || !("Parsing_Time_ms" %in% colnames(donnees))) {
    cat("Scatter plot temps vs précision non généré : colonnes manquantes pour", nom_parseur, "\n")
    return(NULL)
  }
  if (!("Copy_Config" %in% colnames(donnees))) {
    cat("Scatter plot temps vs précision non généré : colonne Copy_Config manquante pour", nom_parseur, "\n")
    return(NULL)
  }
  donnees_valides <- donnees %>% filter(!is.na(Parsing_Time_ms), !is.na(Precision_Error_Avg_px), Precision_Error_Avg_px != -1)
  if (nrow(donnees_valides) == 0) return(NULL)
  dossier_parseur <- paste0("analysis_results/", nom_parseur)

  # Scatter plot complet
  p <- ggplot(donnees_valides, aes(x = Parsing_Time_ms, y = Precision_Error_Avg_px, color = Copy_Config)) +
    geom_point(size = 3, alpha = 0.7) +
    scale_color_brewer(palette = "Set2") +
    labs(title = paste0("Temps vs Précision pour ", nom_parseur),
         subtitle = "Toutes les configurations (tous points, outliers inclus)",
         x = "Temps de parsing (ms)",
         y = "Erreur de précision moyenne (pixels)",
         color = "Config") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    )
  ggsave(paste0(dossier_parseur, "/scatter_temps_vs_precision_", nom_parseur, ".png"), p, width = 10, height = 8, bg = "white")

  # Scatter plot scaled (zoom sur la zone centrale)
  Q1_temps <- quantile(donnees_valides$Parsing_Time_ms, 0.25, na.rm = TRUE)
  Q3_temps <- quantile(donnees_valides$Parsing_Time_ms, 0.75, na.rm = TRUE)
  IQR_temps <- Q3_temps - Q1_temps
  min_temps <- Q1_temps - 1.5 * IQR_temps
  max_temps <- Q3_temps + 1.5 * IQR_temps

  Q1_precision <- quantile(donnees_valides$Precision_Error_Avg_px, 0.25, na.rm = TRUE)
  Q3_precision <- quantile(donnees_valides$Precision_Error_Avg_px, 0.75, na.rm = TRUE)
  IQR_precision <- Q3_precision - Q1_precision
  min_precision <- Q1_precision - 1.5 * IQR_precision
  max_precision <- Q3_precision + 1.5 * IQR_precision

  p_scaled <- p +
    coord_cartesian(xlim = c(min_temps, max_temps), ylim = c(min_precision, max_precision)) +
    labs(subtitle = "Zoom sur la zone centrale (Q1-1.5*IQR à Q3+1.5*IQR)")
  ggsave(paste0(dossier_parseur, "/scatter_temps_vs_precision_", nom_parseur, "_scaled.png"), p_scaled, width = 10, height = 8, bg = "white")

  if (interactive()) {
    print(p)
    print(p_scaled)
  }
  return(list(normal = p, scaled = p_scaled))
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
  
  # Calculer la moyenne et l'écart-type du temps de parsing
  temps_moyen <- mean(donnees$Parsing_Time_ms, na.rm = TRUE)
  ecart_type <- sd(donnees$Parsing_Time_ms, na.rm = TRUE)
    # 1. Graphique des temps d'exécution par copie
  p1 <- ggplot(donnees, aes(x = Numero_Copie, y = Parsing_Time_ms)) +
    geom_line(color = "darkblue", alpha = 0.7) +
    geom_point(color = "darkblue", alpha = 0.7) +
    labs(title = paste("Temps d'exécution par copie -", nom_parseur),
         subtitle = "Évolution du temps de parsing",
         x = "Numéro de copie",
         y = "Temps de parsing (ms)") +
    expand_limits(y = 0) +
    mon_theme
  
  # 3. Histogramme de distribution des temps
  p3 <- ggplot(donnees, aes(x = Parsing_Time_ms)) +
    geom_histogram(bins = 10, fill = "steelblue", color = "darkblue", alpha = 0.7) +
    geom_vline(xintercept = temps_moyen, linetype = "dashed", color = "red", size = 1) +
    labs(title = paste("Distribution des temps de parsing -", nom_parseur),
         subtitle = "Fréquence des différents temps de traitement",
         x = "Temps de parsing (ms)",
         y = "Nombre de copies") +
    expand_limits(x = 0, y = 0) +  # Ajout pour faire apparaître le zéro sur les deux axes
    mon_theme  # 6. Nouvelle analyse - Comparaison des erreurs aux quatre coins
  if (all(c("Precision_Error_TopLeft_px", "Precision_Error_TopRight_px", 
            "Precision_Error_BottomLeft_px", "Precision_Error_BottomRight_px") %in% colnames(donnees))) {
    
    # Transformer les données pour le graphique
    donnees_coins <- donnees %>%
      select(Numero_Copie, 
             TopLeft = Precision_Error_TopLeft_px,
             TopRight = Precision_Error_TopRight_px,
             BottomLeft = Precision_Error_BottomLeft_px,
             BottomRight = Precision_Error_BottomRight_px) %>%
      pivot_longer(cols = c(TopLeft, TopRight, BottomLeft, BottomRight),
                   names_to = "Coin", values_to = "Erreur")
    
    p6 <- ggplot(donnees_coins, aes(x = Coin, y = Erreur, fill = Coin)) +
      geom_boxplot(alpha = 0.7) +
      geom_jitter(width = 0.2, alpha = 0.5) +
      labs(title = paste("Erreur de précision par coin -", nom_parseur),
           subtitle = "Comparaison des quatre coins du marqueur",
           x = "Position",
           y = "Erreur (pixels)") +      scale_fill_brewer(palette = "Set1") +
      expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
      mon_theme
    
    # Créer une version scaled (zoom sur la zone centrale)
    Q1_erreur <- quantile(donnees_coins$Erreur, 0.25, na.rm = TRUE)
    Q3_erreur <- quantile(donnees_coins$Erreur, 0.75, na.rm = TRUE)
    IQR_erreur <- Q3_erreur - Q1_erreur
    min_erreur <- Q1_erreur - 1.5 * IQR_erreur
    max_erreur <- Q3_erreur + 1.5 * IQR_erreur

    p6_scaled <- p6 +
      coord_cartesian(ylim = c(min_erreur, max_erreur)) +
      labs(subtitle = "Zoom sur la zone centrale (Q1-1.5*IQR à Q3+1.5*IQR)")
    
    # Sauvegarder les deux versions du graphique
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_precision_erreur_coins.png"), p6, width = 10, height = 6, bg = "white")
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_precision_erreur_coins_scaled.png"), p6_scaled, width = 10, height = 6, bg = "white")
    
    if (interactive()) {
      print(p6)
      print(p6_scaled)
    }
  }
    # Enregistrer les graphiques dans le dossier du parseurggsave(paste0(dossier_parseur, "/", nom_parseur, "_temps_execution_copies.png"), p1, width = 10, height = 6, bg = "white")
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_distribution_temps.png"), p3, width = 8, height = 6, bg = "white")
  
  if (interactive()) {
    print(p1)
    print(p3)
  }
  
  # Statistiques complémentaires
  cat("\nStatistiques complémentaires pour le parseur", nom_parseur, ":\n")
  cat("Temps moyen de parsing:", mean(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian de parsing:", median(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type:", sd(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Copie la plus rapide:", donnees$File[which.min(donnees$Parsing_Time_ms)], "\n")
  cat("Copie la plus lente:", donnees$File[which.max(donnees$Parsing_Time_ms)], "\n")
  
  # Ajouter les nouvelles statistiques si disponibles
  if ("Precision_Error_Avg_px" %in% colnames(donnees)) {
    cat("Erreur moyenne de précision:", mean(donnees$Precision_Error_Avg_px, na.rm = TRUE), "pixels\n")
  }
  
  if ("Parsing_Success" %in% colnames(donnees)) {
    cat("Taux de succès du parsing:", sum(donnees$Parsing_Success == TRUE, na.rm = TRUE) / nrow(donnees) * 100, "%\n")
  }
  
  # Enregistrer les statistiques dans un fichier texte
  stats_file <- paste0(dossier_parseur, "/", nom_parseur, "_statistiques.txt")
  sink(stats_file)
  cat("Statistiques pour le parseur", nom_parseur, ":\n\n")
  cat("Nombre de copies analysées:", nrow(donnees), "\n")
  cat("Temps moyen de parsing:", mean(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian de parsing:", median(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type:", sd(donnees$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Copie la plus rapide:", donnees$File[which.min(donnees$Parsing_Time_ms)], "\n")
  cat("Copie la plus lente:", donnees$File[which.max(donnees$Parsing_Time_ms)], "\n")
  
  # Ajouter les nouvelles statistiques au fichier
  if ("Precision_Error_Avg_px" %in% colnames(donnees)) {
    cat("Erreur moyenne de précision:", mean(donnees$Precision_Error_Avg_px, na.rm = TRUE), "pixels\n")
    cat("Écart-type de l'erreur de précision:", sd(donnees$Precision_Error_Avg_px, na.rm = TRUE), "pixels\n")
  }
  
  if (all(c("Precision_Error_TopLeft_px", "Precision_Error_TopRight_px", 
            "Precision_Error_BottomLeft_px", "Precision_Error_BottomRight_px") %in% colnames(donnees))) {
    cat("\nErreur moyenne par position:\n")
    cat("- Coin supérieur gauche:", mean(donnees$Precision_Error_TopLeft_px, na.rm = TRUE), "pixels\n")
    cat("- Coin supérieur droit:", mean(donnees$Precision_Error_TopRight_px, na.rm = TRUE), "pixels\n")
    cat("- Coin inférieur gauche:", mean(donnees$Precision_Error_BottomLeft_px, na.rm = TRUE), "pixels\n")
    cat("- Coin inférieur droit:", mean(donnees$Precision_Error_BottomRight_px, na.rm = TRUE), "pixels\n")
  }
  
  if ("Parsing_Success" %in% colnames(donnees)) {
    cat("\nTaux de succès du parsing:", sum(donnees$Parsing_Success == TRUE, na.rm = TRUE) / nrow(donnees) * 100, "%\n")
    cat("Nombre de copies réussies:", sum(donnees$Parsing_Success == TRUE, na.rm = TRUE), "\n")
    cat("Nombre de copies échouées:", sum(donnees$Parsing_Success == FALSE, na.rm = TRUE), "\n")
  }
  
  if ("Copy_Config" %in% colnames(donnees)) {
    cat("\nRépartition par configuration de copie:\n")
    print(table(donnees$Copy_Config))
  }
  
  
  sink()
  
  # Retourner les données pour d'autres analyses potentielles
  generer_barres_precision_par_config(donnees, nom_parseur)
  generer_scatter_temps_vs_precision(donnees, nom_parseur)
  return(donnees)
}

# Fonction pour analyser l'impact des configurations de copie
analyser_impact_config_parseur <- function(donnees, nom_parseur) {
  # Vérifier si la colonne Copy_Config existe
  if (!("Copy_Config" %in% colnames(donnees))) {
    cat("Avertissement : Le fichier CSV ne contient pas la colonne 'Copy_Config'. Analyse des configurations ignorée.\n")
    return(NULL)
  }
  
  # Créer le chemin du dossier pour ce parseur
  dossier_parseur <- paste0("analysis_results/", nom_parseur)
  
  # Statistiques par configuration
  stats_config <- donnees %>%
    group_by(Copy_Config) %>%
    summarise(
      Moyenne_Temps = mean(Parsing_Time_ms, na.rm = TRUE),
      IC_bas_Temps = Moyenne_Temps - qt(0.975, df = n() - 1) * sd(Parsing_Time_ms, na.rm = TRUE) / sqrt(n()),
      IC_haut_Temps = Moyenne_Temps + qt(0.975, df = n() - 1) * sd(Parsing_Time_ms, na.rm = TRUE) / sqrt(n()),
      .groups = "drop"
    )
  
  # Graphique pour le temps de parsing
  p1 <- ggplot() +
    # Points individuels par copie
    geom_point(data = donnees, aes(x = Copy_Config, y = Parsing_Time_ms), 
               color = "darkblue", alpha = 0.6, size = 3, shape = 16) +
    
    # Points moyens
    geom_point(data = stats_config, aes(x = Copy_Config, y = Moyenne_Temps), 
               size = 4, color = "red") +
    
    # Intervalle de confiance
    geom_errorbar(data = stats_config, 
                 aes(x = Copy_Config, ymin = IC_bas_Temps, ymax = IC_haut_Temps), 
                 width = 0.2, color = "red") +
    
    # Titre et axes
    labs(
      title = paste("Impact de la configuration sur le temps de parsing -", nom_parseur),
      subtitle = "Chaque point correspond à une copie individuelle. Moyenne + IC à 95%",
      x = "Configuration de copie",
      y = "Temps de parsing (ms)"
    ) +
    expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(angle = 45, hjust = 1)
    )
  
  # Si la colonne d'erreur de précision existe, faire un graphique similaire
  if ("Precision_Error_Avg_px" %in% colnames(donnees)) {
    stats_config_precision <- donnees %>%
      group_by(Copy_Config) %>%
      summarise(
        Moyenne_Erreur = mean(Precision_Error_Avg_px, na.rm = TRUE),
        IC_bas_Erreur = Moyenne_Erreur - qt(0.975, df = n() - 1) * sd(Precision_Error_Avg_px, na.rm = TRUE) / sqrt(n()),
        IC_haut_Erreur = Moyenne_Erreur + qt(0.975, df = n() - 1) * sd(Precision_Error_Avg_px, na.rm = TRUE) / sqrt(n()),
        .groups = "drop"
      )
    
    p2 <- ggplot() +
      # Points individuels par copie
      geom_point(data = donnees, aes(x = Copy_Config, y = Precision_Error_Avg_px), 
                 color = "darkred", alpha = 0.6, size = 3, shape = 16) +
      
      # Points moyens
      geom_point(data = stats_config_precision, aes(x = Copy_Config, y = Moyenne_Erreur), 
                 size = 4, color = "green4") +
      
      # Intervalle de confiance
      geom_errorbar(data = stats_config_precision, 
                   aes(x = Copy_Config, ymin = IC_bas_Erreur, ymax = IC_haut_Erreur), 
                   width = 0.2, color = "green4") +
      
      # Titre et axes
      labs(
        title = paste("Impact de la configuration sur la précision -", nom_parseur),
        subtitle = "Chaque point correspond à une copie individuelle. Moyenne + IC à 95%",
        x = "Configuration de copie",
        y = "Erreur de précision moyenne (pixels)"
      ) +
      expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
      theme_minimal() +      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        axis.text.x = element_text(angle = 45, hjust = 1)
      )
    
    # Créer une version scaled (zoom sur la zone centrale)
    Q1_erreur <- quantile(donnees$Precision_Error_Avg_px, 0.25, na.rm = TRUE)
    Q3_erreur <- quantile(donnees$Precision_Error_Avg_px, 0.75, na.rm = TRUE)
    IQR_erreur <- Q3_erreur - Q1_erreur
    min_erreur <- Q1_erreur - 1.5 * IQR_erreur
    max_erreur <- Q3_erreur + 1.5 * IQR_erreur

    p2_scaled <- p2 +
      coord_cartesian(ylim = c(min_erreur, max_erreur)) +
      labs(subtitle = "Zoom sur la zone centrale (Q1-1.5*IQR à Q3+1.5*IQR)")
    
    if (interactive()) {
      print(p2)
      print(p2_scaled)
    }
    
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_impact_config_precision.png"), p2, width = 10, height = 7, bg = "white")
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_impact_config_precision_scaled.png"), p2_scaled, width = 10, height = 7, bg = "white")
  }
  
  if (interactive()) {
    print(p1)
  }
  
  ggsave(paste0(dossier_parseur, "/", nom_parseur, "_impact_config_temps.png"), p1, width = 10, height = 7, bg = "white")
  
  # Si on a aussi la colonne de succès du parsing
  if ("Parsing_Success" %in% colnames(donnees)) {
    stats_success <- donnees %>%
      group_by(Copy_Config) %>%
      summarise(
        Taux_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100,
        .groups = "drop"
      )
    
    p3 <- ggplot(stats_success, aes(x = Copy_Config, y = Taux_Succes)) +
      geom_bar(stat = "identity", fill = "steelblue", alpha = 0.8) +
      geom_text(aes(label = paste0(round(Taux_Succes, 1), "%")), vjust = -0.5) +
      labs(
        title = paste("Taux de succès par configuration -", nom_parseur),
        subtitle = "Pourcentage de copies parsées avec succès",
        x = "Configuration de copie",
        y = "Taux de succès (%)"
      ) +
      # Utiliser scale_y_continuous pour garantir le 0 et ajuster le maximum pour les étiquettes
      scale_y_continuous(limits = c(0, max(100, max(stats_success$Taux_Succes, na.rm = TRUE) * 1.1))) +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        axis.text.x = element_text(angle = 45, hjust = 1)
      )
    
    if (interactive()) {
      print(p3)
    }
    
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_taux_succes_config.png"), p3, width = 10, height = 7, bg = "white")
  }
  
  return(stats_config)
}

# Fonction pour analyser les tendances générales du temps de parsing
analyser_tendances_temps_parsing <- function(donnees_completes) {
  cat("\n========== ANALYSE DES TENDANCES GÉNÉRALES DE PARSING ==========\n")
  
  # S'assurer que le dossier existe
  dir_create("analysis_results/informations_generales", recurse = TRUE)
  
  # Vérifier si la colonne Parsing_Time_ms existe
  if (!("Parsing_Time_ms" %in% colnames(donnees_completes))) {
    cat("Avertissement : Le fichier CSV ne contient pas la colonne 'Parsing_Time_ms'. Analyse des tendances de parsing ignorée.\n")
    return(NULL)
  }
  
  # Nettoyer et préparer les données
  # Vérifier les valeurs manquantes ou NA dans Parsing_Time_ms
  nb_na <- sum(is.na(donnees_completes$Parsing_Time_ms))
  if (nb_na > 0) {
    cat("Attention:", nb_na, "valeurs manquantes dans Parsing_Time_ms\n")
  }
  
  # Filtrer les lignes avec des valeurs valides pour Parsing_Time_ms
  donnees_valides <- donnees_completes[!is.na(donnees_completes$Parsing_Time_ms), ]
  
  # Vérifier s'il reste des données après filtrage
  if (nrow(donnees_valides) == 0) {
    cat("Erreur : Aucune donnée valide pour l'analyse des tendances de parsing\n")
    return(NULL)
  }
  
  # Créer un index séquentiel basé sur l'ordre dans le fichier CSV
  donnees_valides$Index_Sequence <- 1:nrow(donnees_valides)
  
  # Afficher quelques informations pour le débogage
  cat("Nombre de lignes avec données valides de parsing:", nrow(donnees_valides), "\n")
  cat("Plage de temps de parsing:", min(donnees_valides$Parsing_Time_ms, na.rm = TRUE), 
      "à", max(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  
  # Définir un thème pour le graphique
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
  
  # Graphique d'évolution du temps de parsing selon l'index séquentiel
  p1 <- ggplot(donnees_valides, aes(x = Index_Sequence, y = Parsing_Time_ms)) +
    geom_point(color = "darkred", alpha = 0.7) +
    geom_smooth(method = "loess", color = "blue", fill = "lightblue", alpha = 0.3) +
    labs(title = "Évolution du temps de parsing",
         subtitle = paste("Tendance générale pour", nrow(donnees_valides), "copies (tous parseurs confondus)"),
         x = "Index séquentiel (ordre dans le fichier CSV)",
         y = "Temps de parsing (ms)") +
    expand_limits(y = 0) +
    mon_theme
  
  # Enregistrer le graphique
  ggsave("analysis_results/informations_generales/evolution_temps_parsing.png", p1, width = 10, height = 6, bg = "white")
  
  if (interactive()) {
    print(p1)
  }
  
  # Statistiques générales sur le temps de parsing
  stats_file <- "analysis_results/informations_generales/statistiques_parsing_general.txt"
  sink(stats_file)
  cat("STATISTIQUES GÉNÉRALES SUR LES TEMPS DE PARSING:\n\n")
  cat("Nombre total de copies analysées:", nrow(donnees_valides), "\n")
  cat("Temps moyen de parsing:", mean(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian de parsing:", median(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type du temps de parsing:", sd(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps de parsing minimum:", min(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps de parsing maximum:", max(donnees_valides$Parsing_Time_ms, na.rm = TRUE), "ms\n")
  
  # Statistiques par type de parseur si disponible
  if ("Parser_Type" %in% colnames(donnees_valides)) {
    cat("\nRépartition par type de parseur:\n")
    stats_parseur <- donnees_valides %>%
      group_by(Parser_Type) %>%
      summarise(
        Nombre = n(),
        Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
        Temps_Median = median(Parsing_Time_ms, na.rm = TRUE),
        Ecart_Type = sd(Parsing_Time_ms, na.rm = TRUE),
        Minimum = min(Parsing_Time_ms, na.rm = TRUE),
        Maximum = max(Parsing_Time_ms, na.rm = TRUE)
      )
    print(stats_parseur)
  }
  sink()
  
  cat("Analyse des tendances générales de parsing terminée. Les résultats sont disponibles dans le dossier 'analysis_results/informations_generales'.\n")
  
  return(TRUE)
}


# Fonction pour analyser les tendances générales
analyser_tendances_generales <- function(donnees_completes) {
  cat("\n========== ANALYSE DES TENDANCES GÉNÉRALES ==========\n")
  
  # Créer un dossier pour les informations générales
  dir_create("analysis_results/informations_generales", recurse = TRUE)
  
  # Vérifier si la colonne Generation_Time_ms existe
  if (!("Generation_Time_ms" %in% colnames(donnees_completes))) {
    cat("Avertissement : Le fichier CSV ne contient pas la colonne 'Generation_Time_ms'. Analyse des tendances de génération ignorée.\n")
    return(NULL)
  }
  
  # Nettoyer et préparer les données
  # Vérifier les valeurs manquantes ou NA dans Generation_Time_ms
  nb_na <- sum(is.na(donnees_completes$Generation_Time_ms))
  if (nb_na > 0) {
    cat("Attention:", nb_na, "valeurs manquantes dans Generation_Time_ms\n")
  }
  
  # Filtrer les lignes avec des valeurs valides pour Generation_Time_ms
  donnees_valides <- donnees_completes[!is.na(donnees_completes$Generation_Time_ms), ]
  
  # Vérifier s'il reste des données après filtrage
  if (nrow(donnees_valides) == 0) {
    cat("Erreur : Aucune donnée valide pour l'analyse des tendances générales\n")
    return(NULL)
  }
  
  # Créer un index séquentiel basé sur l'ordre dans le fichier CSV
  donnees_valides$Index_Sequence <- 1:nrow(donnees_valides)
  
  # Afficher quelques informations pour le débogage
  cat("Nombre de lignes avec données valides:", nrow(donnees_valides), "\n")
  cat("Plage de temps de génération:", min(donnees_valides$Generation_Time_ms, na.rm = TRUE), 
      "à", max(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  
  # Définir un thème pour le graphique
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
  
  # Graphique d'évolution du temps de génération selon l'index séquentiel
  p1 <- ggplot(donnees_valides, aes(x = Index_Sequence, y = Generation_Time_ms)) +
    geom_point(color = "darkgreen", alpha = 0.7) +
    geom_smooth(method = "loess", color = "blue", fill = "lightblue", alpha = 0.3) +
    labs(title = "Évolution du temps de génération",
         subtitle = paste("Tendance générale pour", nrow(donnees_valides), "copies"),
         x = "Index séquentiel (ordre dans le fichier CSV)",
         y = "Temps de génération (ms)") +
    expand_limits(y = 0) +
    mon_theme
  
  # Enregistrer le graphique
  ggsave("analysis_results/informations_generales/evolution_temps_generation.png", p1, width = 10, height = 6, bg = "white")
  
  if (interactive()) {
    print(p1)
  }
  
  # Statistiques générales sur le temps de génération
  stats_file <- "analysis_results/informations_generales/statistiques_generales.txt"
  sink(stats_file)
  cat("STATISTIQUES GÉNÉRALES SUR LES TEMPS DE GÉNÉRATION:\n\n")
  cat("Nombre total de copies analysées:", nrow(donnees_valides), "\n")
  cat("Temps moyen de génération:", mean(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps médian de génération:", median(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  cat("Écart-type du temps de génération:", sd(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps de génération minimum:", min(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  cat("Temps de génération maximum:", max(donnees_valides$Generation_Time_ms, na.rm = TRUE), "ms\n")
  sink()
  
  cat("Analyse des tendances générales terminée. Les résultats sont disponibles dans le dossier 'analysis_results/informations_generales'.\n")
  
  return(TRUE)
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
  if (!all(c("File", "Parsing_Time_ms", "Parser_Type") %in% colnames(donnees_completes))) {
    stop("Erreur : Le fichier CSV ne contient pas les colonnes attendues (File, Parsing_Time_ms, Parser_Type).")
  }
  
  # Créer les dossiers nécessaires
  creer_dossiers()
  analyser_tendances_generales(donnees_completes)
  analyser_tendances_temps_parsing(donnees_completes)
  
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
      
      # Analyser l'impact des configurations de copie si applicable
      stats_config <- analyser_impact_config_parseur(donnees_parseur, parseur)
      
      # Stocker les résultats
      resultats_parseurs[[parseur]] <- list(
        donnees = donnees_analysees,
        stats_config = stats_config
      )
    }
  }
  
  cat("\n========== ANALYSE TERMINÉE ==========\n")
  cat("Les graphiques ont été enregistrés dans le dossier 'analysis_results'.\n")
  
  return(resultats_parseurs)
}

# Point d'entrée du script
main <- function() {
  args <- commandArgs(trailingOnly = TRUE)
  
  if (length(args) == 0) {
    cat("Aucun fichier CSV spécifié. Utilisation du fichier par défaut 'resultats_parseurs.csv'.\n")
    chemin_csv <- "resultats_parseurs.csv"
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