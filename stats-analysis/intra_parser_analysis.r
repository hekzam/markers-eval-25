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
PARSEURS <- c("QRCODE", "CIRCLE", "SHAPE", 
              "ARUCO", "CENTER_MARKER_PARSER")

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
  
  # 1. Graphique linéaire du temps d'exécution par numéro de copie
  p1 <- ggplot(donnees, aes(x = Numero_Copie, y = Parsing_Time_ms)) +
    geom_point(color = "darkblue", size = 3) +
    labs(title = paste("Temps de parsing pour chaque copie -", nom_parseur),
         subtitle = "Évolution du temps de traitement",
         x = "Numéro de copie",
         y = "Temps de parsing (ms)") +
    expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
    mon_theme
  
  # 2. Comparaison avec la moyenne
  p2 <- ggplot(donnees, aes(x = Numero_Copie, y = Parsing_Time_ms)) +
    geom_hline(yintercept = temps_moyen, linetype = "dashed", color = "orangered", size = 1) +
    geom_ribbon(aes(ymin = temps_moyen - ecart_type, ymax = temps_moyen + ecart_type), 
                fill = "lightblue", alpha = 0.3) +
    geom_point(size = 3, aes(color = Parsing_Time_ms > temps_moyen)) +
    scale_color_manual(values = c("darkgreen", "red"),
                       labels = c("Inférieur à la moyenne", "Supérieur à la moyenne"),
                       name = "Performance") +
    labs(title = paste("Comparaison des temps par rapport à la moyenne -", nom_parseur),
         subtitle = paste("Temps moyen:", round(temps_moyen, 5), "ms. Écart-type:", round(ecart_type, 5), "ms"),
         x = "Numéro de copie",
         y = "Temps de parsing (ms)") +
    expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
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
    mon_theme
  
 # 4. Temps d'exécution par groupe de copies (groupé dynamiquement)
  # Calculer la taille de groupe optimale (nombre total de copies / 5)
  taille_groupe <- ceiling(nrow(donnees) / 5)
  
  # S'assurer que la taille de groupe est au moins 1
  taille_groupe <- max(1, taille_groupe)
  
  # Calculer le nombre d'intervalles nécessaires
  max_copie <- max(donnees$Numero_Copie, na.rm = TRUE)
  breaks <- seq(0, max_copie + taille_groupe, by = taille_groupe)
  num_intervals <- length(breaks) - 1
  
  # Créer des groupes de taille dynamique avec le bon nombre d'étiquettes
  donnees$Groupe_Copie <- cut(donnees$Numero_Copie, 
                             breaks = breaks, 
                             include.lowest = TRUE, right = FALSE,
                             labels = paste("Groupe", 1:num_intervals))
  
  p4 <- ggplot(donnees, aes(x = Groupe_Copie, y = Parsing_Time_ms, fill = Groupe_Copie)) +
    geom_boxplot(alpha = 0.7) +
    geom_jitter(width = 0.2, height = 0, size = 2, alpha = 0.7) +
    labs(title = paste("Comparaison des temps de parsing par groupe de copies -", nom_parseur),
         subtitle = paste("Analyse par tranches de", taille_groupe, "copies"),
         x = "Groupe de copies",
         y = "Temps de parsing (ms)") +
    scale_fill_brewer(palette = "Blues") +
    expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
    mon_theme +
    theme(legend.position = "none")
  
  # 5. Nouvelle analyse - Graphique de précision moyenne d'erreur
  if ("Precision_Error_Avg_px" %in% colnames(donnees)) {
    p5 <- ggplot(donnees, aes(x = Numero_Copie, y = Precision_Error_Avg_px)) +
      geom_point(color = "darkred", size = 3) +
      geom_smooth(method = "loess", color = "blue", fill = "lightblue", alpha = 0.3) +
      labs(title = paste("Erreur de précision moyenne par copie -", nom_parseur),
           subtitle = "Évaluation de la précision de détection",
           x = "Numéro de copie",
           y = "Erreur moyenne (pixels)") +
      expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
      mon_theme
    
    # Sauvegarder le graphique d'erreur de précision
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_precision_erreur_moyenne.png"), p5, width = 10, height = 6, bg = "white")
    
    if (interactive()) {
      print(p5)
    }
  }
  
  # 6. Nouvelle analyse - Comparaison des erreurs aux quatre coins
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
           y = "Erreur (pixels)") +
      scale_fill_brewer(palette = "Set1") +
      expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
      mon_theme
    
    # Sauvegarder le graphique de comparaison des coins
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_precision_erreur_coins.png"), p6, width = 10, height = 6, bg = "white")
    
    if (interactive()) {
      print(p6)
    }
  }
  
  # 7. Nouvelle analyse - Taux de succès du parsing
  if ("Parsing_Success" %in% colnames(donnees)) {
    # Convertir en facteur pour assurer le bon affichage
    donnees$Parsing_Success <- as.factor(donnees$Parsing_Success)
    
    # Calculer le pourcentage de succès
    taux_succes <- sum(donnees$Parsing_Success == TRUE, na.rm = TRUE) / nrow(donnees) * 100
    
    p7 <- ggplot(donnees, aes(x = Parsing_Success, fill = Parsing_Success)) +
      geom_bar() +
      geom_text(stat = "count", aes(label = ..count..), vjust = -0.5) +
      labs(title = paste("Taux de succès du parsing -", nom_parseur),
           subtitle = paste0("Pourcentage de succès: ", round(taux_succes, 2), "%"),
           x = "Parsing réussi",
           y = "Nombre de copies") +
      scale_fill_manual(values = c("FALSE" = "red", "TRUE" = "green"),
                      name = "Résultat") +
      expand_limits(y = 0) +  # Ajout pour faire apparaître le zéro sur l'axe Y
      mon_theme
    
    # Sauvegarder le graphique de taux de succès
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_taux_succes.png"), p7, width = 8, height = 6, bg = "white")
    
    if (interactive()) {
      print(p7)
    }
  }
  
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
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        axis.text.x = element_text(angle = 45, hjust = 1)
      )
    
    if (interactive()) {
      print(p2)
    }
    
    ggsave(paste0(dossier_parseur, "/", nom_parseur, "_impact_config_precision.png"), p2, width = 10, height = 7, bg = "white")
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