####################################################
# SCRIPT D'ANALYSE COMPARATIVE INTER-PARSEURS
# Complémentaire au script d'analyse intra-parseur
####################################################

# Chargement des bibliothèques nécessaires
library(ggplot2)
library(dplyr)
library(tidyr)
library(fs)
library(RColorBrewer)  # Pour plus d'options de couleurs
library(gridExtra)     # Pour organiser plusieurs graphiques
#library(fmsb)         # Pour les graphiques en radar (décommenter si nécessaire)

# Liste des parseurs à analyser (la même que dans le script principal)
PARSEURS <- c("QRCODE", "CIRCLE", "SHAPE", 
              "ARUCO", "CENTER_MARKER_PARSER")

# Définir une palette de couleurs pour les parseurs
COULEURS_PARSEURS <- brewer.pal(length(PARSEURS), "Set1")
names(COULEURS_PARSEURS) <- PARSEURS

# Fonction pour créer le dossier de résultats inter-parseurs
creer_dossier_inter_parseurs <- function() {
  dir_create("analysis_results/inter_parseurs", recurse = TRUE)
  cat("Dossier pour l'analyse inter-parseurs créé avec succès\n")
}

# Fonction pour charger et préparer les données
charger_donnees <- function(chemin_csv) {
  # Vérifier si le fichier existe
  if (!file.exists(chemin_csv)) {
    stop("Erreur : Le fichier CSV n'existe pas dans le répertoire indiqué.")
  }
  
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_csv, stringsAsFactors = FALSE)
  
  # Vérifier si les colonnes nécessaires existent
  if (!all(c("File", "Parsing_Time_ms", "Parser_Type") %in% colnames(donnees))) {
    stop("Erreur : Le fichier CSV ne contient pas les colonnes minimales attendues (File, Parsing_Time_ms, Parser_Type).")
  }
  
  # Détecter si les colonnes pour la précision existent
  colonnes_precision <- c("Precision_Error_Avg_px", "Precision_Error_TopLeft_px", 
                         "Precision_Error_TopRight_px", "Precision_Error_BottomLeft_px", 
                         "Precision_Error_BottomRight_px")
  
  has_precision <- any(colonnes_precision %in% colnames(donnees))
  
  # Détecter si la colonne pour le succès du parsing existe
  has_success <- "Parsing_Success" %in% colnames(donnees)
  
  # Détecter si la colonne pour la configuration de copie existe
  has_config <- "Copy_Config" %in% colnames(donnees)
  
  # Retourner les données avec des métadonnées
  return(list(
    donnees = donnees,
    has_precision = has_precision,
    has_success = has_success,
    has_config = has_config
  ))
}

# 1. Boîtes à moustaches comparatives des temps d'exécution par parseur
graphique_boxplot_temps <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Créer le graphique à moustaches (normal, affiche le zéro)
  p <- ggplot(donnees, aes(x = Parser_Type, y = Parsing_Time_ms, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Comparaison des temps d'exécution entre parseurs",
         subtitle = "Distribution des temps de parsing pour chaque parseur",
         x = "Type de parseur",
         y = "Temps de parsing (ms)") +
    expand_limits(y = 0) +  # Affiche le zéro sur l'axe Y
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none"  # Masquer la légende car les couleurs sont redondantes avec l'axe x
    )

  # Créer une version scaled (limite l'axe Y, sans outliers)
  stats_par_parseur <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Q1 = quantile(Parsing_Time_ms, 0.25, na.rm = TRUE),
      Q3 = quantile(Parsing_Time_ms, 0.75, na.rm = TRUE),
      IQR = Q3 - Q1,
      .groups = "drop"
    )
  max_y <- max(stats_par_parseur$Q3 + 1.5 * stats_par_parseur$IQR, na.rm = TRUE)
  p_scaled <- ggplot(donnees, aes(x = Parser_Type, y = Parsing_Time_ms, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7, outlier.shape = NA) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    coord_cartesian(ylim = c(0, max_y)) +
    labs(title = "Comparaison des temps d'exécution entre parseurs (échelle limitée)",
         subtitle = "Distribution des temps de parsing (outliers masqués, axe limité)",
         x = "Type de parseur",
         y = "Temps de parsing (ms)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none"
    )

  # Afficher le graphique principal si en mode interactif
  if (interactive()) {
    print(p)
    print(p_scaled)
  }

  # Enregistrer les deux versions
  ggsave("analysis_results/inter_parseurs/boxplot_temps_execution.png", p, width = 10, height = 6, bg = "white")
  ggsave("analysis_results/inter_parseurs/boxplot_temps_execution_scaled.png", p_scaled, width = 10, height = 6, bg = "white")

  return(list(normal = p, scaled = p_scaled))
}

# 2. Graphique à barres groupées pour comparer la précision ou le taux de succès
graphique_barres_performance <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Préparer une liste pour stocker les graphiques créés
  graphiques <- list()
  
  # Si les données de précision existent, créer un graphique de précision moyenne
  if (donnees_list$has_precision) {
    # Filtrer les valeurs aberrantes (-1)
    donnees_precision <- donnees %>% filter(Precision_Error_Avg_px != -1)
    # Calculer la précision moyenne par parseur
    precision_parseurs <- donnees_precision %>%
      group_by(Parser_Type) %>%
      summarise(
        Erreur_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
        Erreur_Ecart_Type = sd(Precision_Error_Avg_px, na.rm = TRUE),
        .groups = "drop"
      )
    
    # Créer le graphique à barres de précision
    p_precision <- ggplot(precision_parseurs, aes(x = Parser_Type, y = Erreur_Moyenne, fill = Parser_Type)) +
      geom_bar(stat = "identity", alpha = 0.7) +
      geom_errorbar(aes(ymin = Erreur_Moyenne - Erreur_Ecart_Type, 
                       ymax = Erreur_Moyenne + Erreur_Ecart_Type),
                   width = 0.2) +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      labs(title = "Comparaison de l'erreur de précision moyenne entre parseurs",
           subtitle = "Erreur moyenne en pixels avec écart-type",
           x = "Type de parseur",
           y = "Erreur de précision moyenne (pixels)") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none"
      )
    
    # Ajouter à la liste de graphiques
    graphiques$precision <- p_precision
    
    # Enregistrer le graphique
    ggsave("analysis_results/inter_parseurs/barres_erreur_precision.png", p_precision, width = 10, height = 6, bg = "white")
    
    if (interactive()) {
      print(p_precision)
    }
  }
  
  # Si les données de succès existent, créer un graphique de taux de succès
  if (donnees_list$has_success) {
    # Calculer le taux de succès par parseur
    succes_parseurs <- donnees %>%
      group_by(Parser_Type) %>%
      summarise(
        Taux_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100,
        .groups = "drop"
      )
    
    # Créer le graphique à barres de taux de succès
    p_succes <- ggplot(succes_parseurs, aes(x = Parser_Type, y = Taux_Succes, fill = Parser_Type)) +
      geom_bar(stat = "identity", alpha = 0.7) +
      geom_text(aes(label = paste0(round(Taux_Succes, 1), "%")), vjust = -0.5) +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      labs(title = "Comparaison du taux de succès entre parseurs",
           subtitle = "Pourcentage de parsing réussi",
           x = "Type de parseur",
           y = "Taux de succès (%)") +
      scale_y_continuous(limits = c(0, max(100, max(succes_parseurs$Taux_Succes, na.rm = TRUE) * 1.1))) +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none"
      )
    
    # Ajouter à la liste de graphiques
    graphiques$succes <- p_succes
    
    # Enregistrer le graphique
    ggsave("analysis_results/inter_parseurs/barres_taux_succes.png", p_succes, width = 10, height = 6, bg = "white")
    
    if (interactive()) {
      print(p_succes)
    }
    
    # Si la configuration existe aussi, créer un graphique combiné
    if (donnees_list$has_config) {
      # Calculer le taux de succès par parseur et par configuration
      succes_config <- donnees %>%
        group_by(Parser_Type, Copy_Config) %>%
        summarise(
          Taux_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100,
          .groups = "drop"
        )
      
      # Créer le graphique à barres groupées
      p_succes_config <- ggplot(succes_config, aes(x = Copy_Config, y = Taux_Succes, fill = Parser_Type)) +
        geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
        scale_fill_manual(values = COULEURS_PARSEURS) +
        labs(title = "Taux de succès par configuration et par parseur",
             subtitle = "Comparaison du pourcentage de parsing réussi",
             x = "Configuration de copie",
             y = "Taux de succès (%)") +
        scale_y_continuous(limits = c(0, 100)) +
        theme_minimal() +
        theme(
          plot.title = element_text(face = "bold", size = 14, color = "navy"),
          axis.title = element_text(face = "bold", color = "darkblue"),
          axis.text.x = element_text(angle = 45, hjust = 1),
          legend.title = element_text(face = "bold", color = "darkblue"),
          legend.position = "right"
        ) +
        coord_flip()
      ggsave("analysis_results/inter_parseurs/barres_taux_succes_par_config.png", p_succes_config, width = 12, height = 7, bg = "white")
      graphiques$succes_config <- p_succes_config
    }
  }
  
  return(graphiques)
}

# 3. Graphiques linéaires pour comparer l'évolution des performances
graphique_evolution_performances <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Si la colonne de configuration existe, on peut l'utiliser comme variable continue
  if (donnees_list$has_config) {
    # Essayer de convertir la configuration en valeur numérique si elle contient des nombres
    if (all(grepl("^[0-9]+$", unique(donnees$Copy_Config)))) {
      donnees$Config_Num <- as.numeric(donnees$Copy_Config)
    } else {
      # Si non numérique, utiliser un facteur ordonné
      donnees$Config_Num <- as.numeric(factor(donnees$Copy_Config))
    }
    # Suppression de la génération des graphes evolution_temps_par_config et evolution_precision_par_config
    return(NULL)
  }
  return(NULL)
}

# 4. Scatter plot pour explorer la relation entre temps et précision
graphique_scatter_temps_precision <- function(donnees_list) {
  donnees <- donnees_list$donnees

  if (!donnees_list$has_precision) {
    cat("Scatter plot temps vs précision non disponible : données de précision manquantes\n")
    return(NULL)
  }
  if (!donnees_list$has_config) {
    cat("Scatter plot par parseur/config non disponible : données de configuration manquantes\n")
    return(NULL)
  }

  # On ne filtre plus les outliers, mais on les garde pour la version complète
  donnees_non_outliers <- list()
  plots <- list()
  plots_scaled <- list()

  for (parseur in unique(donnees$Parser_Type)) {
    donnees_parseur <- donnees %>% filter(Parser_Type == parseur & Precision_Error_Avg_px != -1)
    if (nrow(donnees_parseur) == 0) next

    # Calcul des bornes pour la version zoomée
    Q1_temps <- quantile(donnees_parseur$Parsing_Time_ms, 0.25, na.rm = TRUE)
    Q3_temps <- quantile(donnees_parseur$Parsing_Time_ms, 0.75, na.rm = TRUE)
    IQR_temps <- Q3_temps - Q1_temps
    min_temps <- Q1_temps - 1.5 * IQR_temps
    max_temps <- Q3_temps + 1.5 * IQR_temps

    Q1_precision <- quantile(donnees_parseur$Precision_Error_Avg_px, 0.25, na.rm = TRUE)
    Q3_precision <- quantile(donnees_parseur$Precision_Error_Avg_px, 0.75, na.rm = TRUE)
    IQR_precision <- Q3_precision - Q1_precision
    min_precision <- Q1_precision - 1.5 * IQR_precision
    max_precision <- Q3_precision + 1.5 * IQR_precision

    # Plot complet (tous les points)
    p <- ggplot(donnees_parseur, aes(x = Parsing_Time_ms, y = Precision_Error_Avg_px, color = Copy_Config)) +
      geom_point(size = 3, alpha = 0.7) +
      scale_color_brewer(palette = "Set2") +
      labs(title = paste0("Temps vs Précision pour ", parseur),
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
    ggsave(paste0("analysis_results/inter_parseurs/scatter_temps_vs_precision_", parseur, ".png"), p, width = 10, height = 8, bg = "white")
    plots[[parseur]] <- p

    # Plot zoomé (scaled)
    p_scaled <- p +
      coord_cartesian(xlim = c(min_temps, max_temps), ylim = c(min_precision, max_precision)) +
      labs(subtitle = "Zoom sur la zone centrale (Q1-1.5*IQR à Q3+1.5*IQR)")
    ggsave(paste0("analysis_results/inter_parseurs/scatter_temps_vs_precision_", parseur, "_scaled.png"), p_scaled, width = 10, height = 8, bg = "white")
    plots_scaled[[parseur]] <- p_scaled
  }
  if (interactive()) {
    for (p in plots) print(p)
    for (p in plots_scaled) print(p)
  }
  return(list(plots = plots, plots_scaled = plots_scaled))
}

# 5. Violin plots pour comparer la distribution des temps d'exécution
graphique_violin_temps <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Créer le violin plot
  p <- ggplot(donnees, aes(x = Parser_Type, y = Parsing_Time_ms, fill = Parser_Type)) +
    geom_violin(trim = FALSE, alpha = 0.7) +
    geom_boxplot(width = 0.1, fill = "white", alpha = 0.5) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Distribution des temps d'exécution par parseur",
         subtitle = "Visualisation de la densité des temps de parsing",
         x = "Type de parseur",
         y = "Temps de parsing (ms)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none"
    )
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/violin_temps_execution.png", p, width = 10, height = 6, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
  # Si les données de précision existent, créer aussi un violin plot pour la précision
  if (donnees_list$has_precision) {
    # Calculer des statistiques sur les erreurs de précision pour détecter les valeurs aberrantes
    Q1 <- quantile(donnees$Precision_Error_Avg_px, 0.25, na.rm = TRUE)
    Q3 <- quantile(donnees$Precision_Error_Avg_px, 0.75, na.rm = TRUE)
    IQR <- Q3 - Q1
    
    # Définir des seuils pour les valeurs aberrantes (1.5 × IQR est un seuil standard)
    seuil_inf <- max(0, Q1 - 1.5 * IQR)  # On ne veut pas de valeurs négatives pour l'erreur
    seuil_sup <- Q3 + 1.5 * IQR
    
    # Filtrer les données pour le graphique de précision
    donnees_filtrees <- donnees %>%
      filter(Precision_Error_Avg_px >= seuil_inf & Precision_Error_Avg_px <= seuil_sup)
    
    # Calculer combien de points ont été filtrés
    nb_points_filtres <- nrow(donnees) - nrow(donnees_filtrees)
    pourcentage_filtre <- round(nb_points_filtres / nrow(donnees) * 100, 1)
    
    # Message pour l'utilisateur
    cat(paste0("Violin plot de précision : ", nb_points_filtres, " points aberrants filtrés (", 
               pourcentage_filtre, "% des données)\n"))
    
    # Créer le violin plot SANS filtrage (toutes les valeurs)
    p2 <- ggplot(donnees, aes(x = Parser_Type, y = Precision_Error_Avg_px, fill = Parser_Type)) +
      geom_violin(trim = FALSE, alpha = 0.7) +
      geom_boxplot(width = 0.1, fill = "white", alpha = 0.5) +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      labs(title = "Distribution des erreurs de précision par parseur",
           subtitle = "Toutes les valeurs (aucun filtrage)",
           x = "Type de parseur",
           y = "Erreur de précision (pixels)") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none"
      )

    # Enregistrer le graphique non filtré
    ggsave("analysis_results/inter_parseurs/violin_erreur_precision.png", p2, width = 10, height = 6, bg = "white")

    # Créer aussi une version avec ylim pour montrer l'ensemble des données mais avec une limite visuelle (scaled)
    p3 <- ggplot(donnees, aes(x = Parser_Type, y = Precision_Error_Avg_px, fill = Parser_Type)) +
      geom_violin(trim = TRUE, alpha = 0.7) +  # trim=TRUE pour couper les extrémités
      geom_boxplot(width = 0.1, fill = "white", alpha = 0.5, outlier.shape = NA) +  # masquer les outliers
      scale_fill_manual(values = COULEURS_PARSEURS) +
      coord_cartesian(ylim = c(0, Q3 + 1.5 * IQR)) +  # Limiter l'axe y sans perdre les données
      labs(title = "Distribution des erreurs de précision par parseur",
           subtitle = "Échelle limitée pour une meilleure visualisation",
           x = "Type de parseur",
           y = "Erreur de précision (pixels)") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none"
      )

    # Enregistrer la version avec ylim
    ggsave("analysis_results/inter_parseurs/violin_erreur_precision_scaled.png", p3, width = 10, height = 6, bg = "white")

    if (interactive()) {
      print(p2)
      print(p3)
    }
    
    return(list(temps = p, precision_filtree = p2, precision_scalee = p3))
  }
  
  return(list(temps = p))
}

# 6. Front de Pareto global : temps/précision pour chaque (Parser_Type, Copy_Config)
# Un point = une combinaison unique (Parser_Type, Copy_Config)
graphique_pareto_temps_global <- function(donnees_list, lisser = FALSE, scaled = FALSE) {
  donnees <- donnees_list$donnees
  if (!donnees_list$has_precision) {
    cat("Front de Pareto non disponible : données de précision manquantes\n")
    return(NULL)
  }
  # On ne garde que les lignes valides
  donnees <- donnees %>% filter(!is.na(Parsing_Time_ms), !is.na(Precision_Error_Avg_px), Precision_Error_Avg_px != -1)

  # Un point par combinaison unique (Parser_Type, Copy_Config)
  stats <- donnees %>%
    group_by(Parser_Type, Copy_Config) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      Precision_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
      Parsing_Success = mean(Parsing_Success, na.rm = TRUE),
      .groups = "drop"
    )
  stats$Label <- paste0(stats$Parser_Type, " | ", stats$Copy_Config)

  # Calcul du front de Pareto (minimiser X et Y)
  is_dominated <- function(i) {
    any(stats$Temps_Moyen < stats$Temps_Moyen[i] & stats$Precision_Moyenne < stats$Precision_Moyenne[i])
  }
  front <- !sapply(1:nrow(stats), is_dominated)
  stats$Pareto <- ifelse(front, "Front de Pareto", "Dominé")

  # Coloration : Parsing_Success si dispo, sinon Parser_Type
  color_col <- if ("Parsing_Success" %in% colnames(stats)) "Parsing_Success" else "Parser_Type"

  p <- ggplot(stats, aes(x = Temps_Moyen, y = Precision_Moyenne, color = .data[[color_col]], shape = Pareto, label = Label)) +
    geom_point(size = 3, alpha = 0.8) +
    geom_text(vjust = -1, size = 2.5, check_overlap = TRUE) +
    scale_color_gradient(low = "grey", high = "green", na.value = "red") +
    labs(title = ifelse(scaled, "Front de Pareto global (échelle limitée) : Temps vs Précision", "Front de Pareto global : Temps vs Précision"),
         x = "Temps moyen de parsing (ms)",
         y = "Erreur de précision moyenne (px, plus bas = mieux)",
         color = ifelse(color_col == "Parsing_Success", "Taux de succès", "Parseur"),
         shape = "Statut") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "top"
    )

  # Relier les points du front de Pareto
  pareto_points <- stats[front, ]
  if (nrow(pareto_points) > 1) {
    pareto_points <- pareto_points[order(pareto_points$Temps_Moyen, decreasing = FALSE), ]
    p <- p + geom_path(data = pareto_points, aes(x = Temps_Moyen, y = Precision_Moyenne), color = "red", size = 1.2, inherit.aes = FALSE)
  }

  # Version scaled : limite l'axe X et Y à Q3+1.5*IQR
  if (scaled) {
    Q1_x <- quantile(stats$Temps_Moyen, 0.25, na.rm = TRUE)
    Q3_x <- quantile(stats$Temps_Moyen, 0.75, na.rm = TRUE)
    IQR_x <- Q3_x - Q1_x
    max_x <- Q3_x + 1.5 * IQR_x
    min_x <- max(0, Q1_x - 1.5 * IQR_x)
    Q1_y <- quantile(stats$Precision_Moyenne, 0.25, na.rm = TRUE)
    Q3_y <- quantile(stats$Precision_Moyenne, 0.75, na.rm = TRUE)
    IQR_y <- Q3_y - Q1_y
    max_y <- Q3_y + 1.5 * IQR_y
    min_y <- max(0, Q1_y - 1.5 * IQR_y)
    p <- p + coord_cartesian(xlim = c(min_x, max_x), ylim = c(min_y, max_y))
  }

  nom_fichier <- ifelse(scaled, "analysis_results/inter_parseurs/front_pareto_global_scaled.png", "analysis_results/inter_parseurs/front_pareto_global.png")
  ggsave(nom_fichier, p, width = 13, height = 8, bg = "white")
  if (interactive()) print(p)
  return(p)
}

# 6c. Pareto optimisation (front de Pareto) temps/précision ou temps/fiabilité pour tous les couples parseur/config
# Un point par (parseur, config), front de Pareto global

graphique_pareto_temps_parseur_config <- function(donnees_list) {
  donnees <- donnees_list$donnees
  if (!donnees_list$has_config) {
    cat("Front de Pareto global parseur/config non disponible : données de configuration manquantes\n")
    return(NULL)
  }
  # Calculer les moyennes par couple (parseur, config)
  stats <- donnees %>%
    group_by(Parser_Type, Copy_Config) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      Precision_Moyenne = if ("Precision_Error_Avg_px" %in% colnames(donnees)) mean(Precision_Error_Avg_px[Precision_Error_Avg_px != -1], na.rm = TRUE) else NA,
      Fiabilite = if ("Parsing_Success" %in% colnames(donnees)) sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100 else NA,
      .groups = "drop"
    )
  # Choix des axes : priorité à la précision si dispo, sinon fiabilité
  if (!all(is.na(stats$Precision_Moyenne))) {
    xvar <- "Temps_Moyen"
    yvar <- "Precision_Moyenne"
    xlabel <- "Temps moyen de parsing (ms)"
    ylabel <- "Erreur de précision moyenne (px, plus bas = mieux)"
    titre <- "Front de Pareto global : Temps vs Précision (tous parseurs/configs)"
    # Plus bas = mieux pour les deux axes
    is_dominated <- function(i) {
      any(stats$Temps_Moyen < stats$Temps_Moyen[i] & stats$Precision_Moyenne < stats$Precision_Moyenne[i])
    }
    front <- !sapply(1:nrow(stats), is_dominated)
  } else if (!all(is.na(stats$Fiabilite))) {
    xvar <- "Temps_Moyen"
    yvar <- "Fiabilite"
    xlabel <- "Temps moyen de parsing (ms)"
    ylabel <- "Taux de succès (%)"
    titre <- "Front de Pareto global : Temps vs Fiabilité (tous parseurs/configs)"
    # Plus bas temps, plus haut taux de succès
    is_dominated <- function(i) {
      any(stats$Temps_Moyen < stats$Temps_Moyen[i] & stats$Fiabilite > stats$Fiabilite[i])
    }
    front <- !sapply(1:nrow(stats), is_dominated)
  } else {
    stop("Impossible de générer le front de Pareto : ni précision ni fiabilité disponibles.")
  }
  stats$Pareto <- ifelse(front, "Front de Pareto", "Dominé")
  # Pour l'affichage, concaténer parseur et config
  stats$Label <- paste0(stats$Parser_Type, " | ", stats$Copy_Config)
  # Plot
  p <- ggplot(stats, aes_string(x = xvar, y = yvar, color = "Pareto", label = "Label")) +
    geom_point(size = 4, alpha = 0.8) +
    geom_text(vjust = -1, fontface = "bold", size = 3) +
    scale_color_manual(values = c("Front de Pareto" = "red", "Dominé" = "grey")) +
    labs(title = titre, x = xlabel, y = ylabel, color = "Statut") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "top"
    )
  # Relier les points du front de Pareto
  pareto_points <- stats[front, ]
  if (nrow(pareto_points) > 1) {
    pareto_points <- pareto_points[order(pareto_points[[xvar]], decreasing = FALSE), ]
    p <- p + geom_path(data = pareto_points, aes_string(x = xvar, y = yvar), color = "red", size = 1.2)
  }
  ggsave("analysis_results/inter_parseurs/pareto_temps_moyens_global_parseur_config.png", p, width = 13, height = 8, bg = "white")
  if (interactive()) print(p)
  return(p)
}

# 7. Comparaison des erreurs par coin entre les parseurs - CORRIGÉE
graphique_erreurs_coins <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Vérifier si toutes les colonnes nécessaires existent
  colonnes_coins <- c("Precision_Error_TopLeft_px", "Precision_Error_TopRight_px", 
                     "Precision_Error_BottomLeft_px", "Precision_Error_BottomRight_px")
  
  if (!all(colonnes_coins %in% colnames(donnees))) {
    cat("Graphique des erreurs par coin non disponible : colonnes d'erreur par coin manquantes\n")
    return(NULL)
  }
  
  # Transformer les données pour le graphique
  donnees_coins <- donnees %>%
    select(Parser_Type, 
           TopLeft = Precision_Error_TopLeft_px,
           TopRight = Precision_Error_TopRight_px,
           BottomLeft = Precision_Error_BottomLeft_px,
           BottomRight = Precision_Error_BottomRight_px) %>%
    pivot_longer(cols = c(TopLeft, TopRight, BottomLeft, BottomRight),
                 names_to = "Coin", values_to = "Erreur")
  
  # Filtrer les valeurs aberrantes pour chaque coin et parseur
  # Calcul des quartiles par coin et parseur
  stats_par_groupe <- donnees_coins %>%
    group_by(Parser_Type, Coin) %>%
    summarise(
      Q1 = quantile(Erreur, 0.25, na.rm = TRUE),
      Q3 = quantile(Erreur, 0.75, na.rm = TRUE),
      IQR = Q3 - Q1,
      .groups = "drop"
    )
  
  # Joindre avec les données originales
  donnees_coins_avec_limites <- donnees_coins %>%
    left_join(stats_par_groupe, by = c("Parser_Type", "Coin")) %>%
    mutate(
      seuil_inf = max(0, Q1 - 1.5 * IQR),  # Pas de valeurs négatives pour l'erreur
      seuil_sup = Q3 + 1.5 * IQR
    )
  
  # Filtrer les données
  donnees_coins_filtrees <- donnees_coins_avec_limites %>%
    filter(Erreur >= seuil_inf & Erreur <= seuil_sup)
  
  # Calculer le nombre de points filtrés
  nb_points_filtres <- nrow(donnees_coins) - nrow(donnees_coins_filtrees)
  pourcentage_filtre <- round(nb_points_filtres / nrow(donnees_coins) * 100, 1)
  
  cat(paste0("Graphique des erreurs par coin : ", nb_points_filtres, " points aberrants filtrés (", 
             pourcentage_filtre, "% des données)\n"))
  
  # Créer le graphique SANS filtrage des valeurs aberrantes
  p <- ggplot(donnees_coins, aes(x = Coin, y = Erreur, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Comparaison des erreurs de précision par coin entre parseurs",
         subtitle = "Distribution des erreurs pour chaque coin (aucun filtrage)",
         x = "Position",
         y = "Erreur (pixels)",
         fill = "Parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    )

  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/comparaison_erreurs_coins.png", p, width = 12, height = 7, bg = "white")

  if (interactive()) {
    print(p)
  }
  
  # Créer également un graphique non filtré mais avec échelle ajustée
  p_scaled <- ggplot(donnees_coins, aes(x = Coin, y = Erreur, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7, outlier.shape = NA) +  # Masquer les outliers
    coord_cartesian(ylim = c(0, max(stats_par_groupe$Q3 + 1.5 * stats_par_groupe$IQR))) +  # Limiter l'axe y sans perdre les données
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Comparaison des erreurs de précision par coin entre parseurs",
         subtitle = "Échelle limitée pour une meilleure visualisation",
         x = "Position",
         y = "Erreur (pixels)",
         fill = "Parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    )
  
  # Enregistrer le graphique à échelle ajustée
  ggsave("analysis_results/inter_parseurs/comparaison_erreurs_coins_scaled.png", p_scaled, width = 12, height = 7, bg = "white")
  
  if (interactive()) {
    print(p_scaled)
  }
  
  # Créer également un graphique avec les valeurs moyennes
  # Filtrer les Erreur == -1 pour le barplot des moyennes
  moyennes_coins <- donnees_coins %>%
    filter(Erreur != -1) %>%
    group_by(Parser_Type, Coin) %>%
    summarise(
      Erreur_Moyenne = mean(Erreur, na.rm = TRUE),
      .groups = "drop"
    )
  
  p2 <- ggplot(moyennes_coins, aes(x = Coin, y = Erreur_Moyenne, fill = Parser_Type)) +
    geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Erreur moyenne par coin et par parseur",
         subtitle = "Comparaison de la précision à chaque coin du marqueur",
         x = "Position",
         y = "Erreur moyenne (pixels)",
         fill = "Parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    )
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/moyennes_erreurs_coins.png", p2, width = 12, height = 7, bg = "white")
  
  if (interactive()) {
    print(p2)
  }
  
  return(list(boxplot_filtre = p, boxplot_scale = p_scaled, barres = p2))
}

# 8. Tableau récapitulatif des performances
generer_tableau_performances <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Préparer le tableau des performances de base (temps)
  tableau_temps <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Temps_Min = round(min(Parsing_Time_ms, na.rm = TRUE), 2),
      Temps_Q1 = round(quantile(Parsing_Time_ms, 0.25, na.rm = TRUE), 2),
      Temps_Median = round(median(Parsing_Time_ms, na.rm = TRUE), 2),
      Temps_Moyen = round(mean(Parsing_Time_ms, na.rm = TRUE), 2),
      Temps_Q3 = round(quantile(Parsing_Time_ms, 0.75, na.rm = TRUE), 2),
      Temps_Max = round(max(Parsing_Time_ms, na.rm = TRUE), 2),
      Temps_Ecart_Type = round(sd(Parsing_Time_ms, na.rm = TRUE), 2),
      .groups = "drop"
    )
  
  # Si disponible, ajouter les données de précision
  if (donnees_list$has_precision) {
    tableau_precision <- donnees %>%
      group_by(Parser_Type) %>%
      summarise(
        Precision_Min = round(min(Precision_Error_Avg_px, na.rm = TRUE), 2),
        Precision_Q1 = round(quantile(Precision_Error_Avg_px, 0.25, na.rm = TRUE), 2),
        Precision_Median = round(median(Precision_Error_Avg_px, na.rm = TRUE), 2),
        Precision_Moyen = round(mean(Precision_Error_Avg_px, na.rm = TRUE), 2),
        Precision_Q3 = round(quantile(Precision_Error_Avg_px, 0.75, na.rm = TRUE), 2),
        Precision_Max = round(max(Precision_Error_Avg_px, na.rm = TRUE), 2),
        Precision_Ecart_Type = round(sd(Precision_Error_Avg_px, na.rm = TRUE), 2),
        .groups = "drop"
      )
    
    # Fusionner avec le tableau de temps
    tableau_performances <- left_join(tableau_temps, tableau_precision, by = "Parser_Type")
  } else {
    tableau_performances <- tableau_temps
  }
  
  # Si disponible, ajouter les données de succès
  if (donnees_list$has_success) {
    tableau_succes <- donnees %>%
      group_by(Parser_Type) %>%
      summarise(
        Taux_Succes = paste0(round(sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100, 2), "%"),
        Nb_Total = n(),
        Nb_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE),
        Nb_Echec = sum(Parsing_Success == FALSE, na.rm = TRUE),
        .groups = "drop"
      )
    
    # Fusionner avec le tableau de performances
    tableau_performances <- left_join(tableau_performances, tableau_succes, by = "Parser_Type")
  }
  
  # Enregistrer le tableau dans un fichier CSV
  write.csv(tableau_performances, "analysis_results/inter_parseurs/tableau_performances.csv", row.names = FALSE)
  
  # Afficher le tableau dans la console
  cat("Tableau récapitulatif des performances par parseur :\n")
  print(tableau_performances)
  
  return(tableau_performances)
}

# 9. Triple boxplot pour comparer les performances globales
# Remplace le radar plot par trois boxplots côte à côte (Rapidité, Précision, Fiabilité)
graphique_radar_performance <- function(donnees_list) {
  donnees <- donnees_list$donnees

  if (!donnees_list$has_precision) {
    cat("Triple boxplot non disponible : données de précision manquantes\n")
    return(NULL)
  }

  # Liste des parseurs présents dans les données
  parseurs <- unique(donnees$Parser_Type)
  result_plots <- list()

  for (parseur in parseurs) {
    donnees_parseur <- donnees %>% filter(Parser_Type == parseur)
    if (nrow(donnees_parseur) == 0) next

    donnees_long <- donnees_parseur %>%
      mutate(Succes_Pourcent = if ("Parsing_Success" %in% colnames(donnees_parseur)) as.numeric(Parsing_Success) * 100 else NA) %>%
      select(Parser_Type, Parsing_Time_ms, Precision_Error_Avg_px, Succes_Pourcent) %>%
      pivot_longer(
        cols = c(Parsing_Time_ms, Precision_Error_Avg_px, Succes_Pourcent),
        names_to = "Metrique", values_to = "Valeur"
      )

    donnees_long$Metrique <- factor(
      donnees_long$Metrique,
      levels = c("Parsing_Time_ms", "Precision_Error_Avg_px", "Succes_Pourcent"),
      labels = c("Rapidité (ms, plus bas = mieux)", "Précision (px, plus bas = mieux)", "Fiabilité (%)")
    )
    donnees_long <- donnees_long[!is.na(donnees_long$Valeur), ]

    p <- ggplot(donnees_long, aes(x = Metrique, y = Valeur, fill = Metrique)) +
      geom_boxplot(alpha = 0.7, outlier.shape = NA) +
      scale_fill_brewer(palette = "Set1") +
      labs(title = paste0("Triple boxplot des performances - ", parseur),
           subtitle = "Rapidité (ms), Précision (px, plus bas = mieux)", "Fiabilité (%)",
           x = "Métrique", y = "Valeur") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none",
        strip.text = element_text(face = "bold", size = 12)
      )

    chemin <- paste0("analysis_results/inter_parseurs/triple_boxplot_", parseur, ".png")
    ggsave(chemin, p, width = 8, height = 6, bg = "white")
    result_plots[[parseur]] <- p
  }

  return(result_plots)
}

# 10. Graphique combiné pour visualiser les performances par configuration et par parseur
graphique_perf_par_config <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Vérifier si les données de configuration existent
  if (!donnees_list$has_config) {
    cat("Graphique de performance par configuration non disponible : données de configuration manquantes\n")
    return(NULL)
  }
  
  # Calculer des statistiques par configuration et parseur
  stats_config <- donnees %>%
    group_by(Parser_Type, Copy_Config) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      Temps_Median = median(Parsing_Time_ms, na.rm = TRUE),
      Temps_Ecart_Type = sd(Parsing_Time_ms, na.rm = TRUE),
      .groups = "drop"
    )
  
  # Si les données de précision existent, ajouter ces métriques
  if (donnees_list$has_precision) {
    # Filtrer les valeurs aberrantes (-1)
    donnees_precision <- donnees %>% filter(Precision_Error_Avg_px != -1)
    precision_config <- donnees_precision %>%
      group_by(Parser_Type, Copy_Config) %>%
      summarise(
        Erreur_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
        Erreur_Median = median(Precision_Error_Avg_px, na.rm = TRUE),
        Erreur_Ecart_Type = sd(Precision_Error_Avg_px, na.rm = TRUE),
        .groups = "drop"
      )
    stats_config <- left_join(stats_config, precision_config, by = c("Parser_Type", "Copy_Config"))
  }
  
  # Si les données de succès existent, ajouter ces métriques
  if (donnees_list$has_success) {
    succes_config <- donnees %>%
      group_by(Parser_Type, Copy_Config) %>%
      summarise(
        Taux_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100,
        .groups = "drop"
      )
    
    stats_config <- left_join(stats_config, succes_config, by = c("Parser_Type", "Copy_Config"))
  }
  
  # Graphique en grille pour les temps moyens par configuration
  p_temps <- ggplot(stats_config, aes(x = Copy_Config, y = Temps_Moyen, fill = Parser_Type)) +
    geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
    geom_errorbar(aes(ymin = Temps_Moyen - Temps_Ecart_Type, 
                     ymax = Temps_Moyen + Temps_Ecart_Type),
                 position = position_dodge(width = 0.9), width = 0.2) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Temps moyen de parsing par configuration",
         subtitle = "Avec barres d'erreur représentant l'écart-type",
         x = "Configuration",
         y = "Temps moyen (ms)",
         fill = "Parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(angle = 45, hjust = 1),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    ) +
    coord_flip()
  
  # Enregistrer le graphique des temps
  ggsave("analysis_results/inter_parseurs/barres_temps_par_config.png", p_temps, width = 12, height = 7, bg = "white")
  
  graphiques <- list(temps = p_temps)
  
  # Si les données de précision existent, créer le graphique correspondant
  if (donnees_list$has_precision) {
    p_precision <- ggplot(stats_config, aes(x = Copy_Config, y = Erreur_Moyenne, fill = Parser_Type)) +
      geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
      geom_errorbar(aes(ymin = Erreur_Moyenne - Erreur_Ecart_Type, 
                       ymax = Erreur_Moyenne + Erreur_Ecart_Type),
                   position = position_dodge(width = 0.9), width = 0.2) +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      labs(title = "Erreur moyenne de précision par configuration",
           subtitle = "Avec barres d'erreur représentant l'écart-type",
           x = "Configuration",
           y = "Erreur moyenne (pixels)",
           fill = "Parseur") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        axis.text.x = element_text(angle = 45, hjust = 1),
        legend.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "right"
      ) +
      coord_flip()
    
    # Enregistrer le graphique de précision global
    ggsave("analysis_results/inter_parseurs/barres_precision_par_config.png", p_precision, width = 12, height = 7, bg = "white")
    graphiques$precision <- p_precision

    # Générer un graphique par parseur
    for (parser in unique(stats_config$Parser_Type)) {
      stats_config_parser <- stats_config %>% filter(Parser_Type == parser)
      p_precision_parser <- ggplot(stats_config_parser, aes(x = Copy_Config, y = Erreur_Moyenne, fill = Copy_Config)) +
        geom_bar(stat = "identity", position = "dodge", alpha = 0.8) +
        geom_errorbar(aes(ymin = Erreur_Moyenne - Erreur_Ecart_Type, 
                         ymax = Erreur_Moyenne + Erreur_Ecart_Type),
                     position = position_dodge(width = 0.9), width = 0.2) +
        scale_fill_brewer(palette = "Set2") +
        labs(title = paste0("Erreur moyenne de précision par configuration - ", parser),
             subtitle = "Avec barres d'erreur représentant l'écart-type",
             x = NULL, # Pas de label sous le graphique
             y = "Erreur moyenne (pixels)",
             fill = "Configuration") +
        theme_minimal() +
        theme(
          plot.title = element_text(face = "bold", size = 14, color = "navy"),
          axis.title = element_text(face = "bold", color = "darkblue"),
          axis.text.x = element_blank(), # Masque les labels de configuration
          axis.ticks.x = element_blank(), # Masque les ticks
          legend.title = element_text(face = "bold", color = "darkblue"),
          legend.position = "right"
        )
      # PAS de coord_flip ici
      ggsave(paste0("analysis_results/inter_parseurs/barres_precision_par_config_", parser, ".png"), p_precision_parser, width = 12, height = 7, bg = "white")
      graphiques[[paste0("precision_", parser)]] <- p_precision_parser
    }
  }
  
  if (interactive()) {
    for (g in graphiques) {
      print(g)
    }
  }
  
  return(graphiques)
}

# 7. Graphique avec courbes gaussiennes pour la distribution des temps et erreurs
# (modifié : suppression de la courbe gaussienne théorique)
graphique_courbes_gaussiennes <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # --- Graphique pour les temps d'exécution : SANS filtrage ---
  p_temps <- ggplot(donnees, aes(x = Parsing_Time_ms, fill = Parser_Type, color = Parser_Type)) +
    geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
    geom_density(alpha = 0.6) +
    facet_wrap(~ Parser_Type, scales = "free") +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    scale_color_manual(values = COULEURS_PARSEURS) +
    labs(title = "Distribution des temps d'exécution par parseur",
         subtitle = "Distribution réelle (aucun filtrage)",
         x = "Temps de parsing (ms)",
         y = "Densité") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none",
      strip.text = element_text(face = "bold", size = 12)
    )

  # Version scaled (axe limité, outliers masqués)
  stats_par_parseur <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Q1 = quantile(Parsing_Time_ms, 0.25, na.rm = TRUE),
      Q3 = quantile(Parsing_Time_ms, 0.75, na.rm = TRUE),
      IQR = Q3 - Q1,
      .groups = "drop"
    )
  max_y_temps <- max(stats_par_parseur$Q3 + 1.5 * stats_par_parseur$IQR, na.rm = TRUE)
  p_temps_scaled <- ggplot(donnees, aes(x = Parsing_Time_ms, fill = Parser_Type, color = Parser_Type)) +
    geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
    geom_density(alpha = 0.6) +
    facet_wrap(~ Parser_Type, scales = "free") +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    scale_color_manual(values = COULEURS_PARSEURS) +
    coord_cartesian(xlim = c(0, max_y_temps)) +
    labs(title = "Distribution des temps d'exécution par parseur (échelle limitée)",
         subtitle = "Distribution réelle (axe limité, outliers masqués)",
         x = "Temps de parsing (ms)",
         y = "Densité") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none",
      strip.text = element_text(face = "bold", size = 12)
    )

  ggsave("analysis_results/inter_parseurs/distribution_gaussienne_temps.png", p_temps, width = 12, height = 8, bg = "white")
  ggsave("analysis_results/inter_parseurs/distribution_gaussienne_temps_scaled.png", p_temps_scaled, width = 12, height = 8, bg = "white")

  if (interactive()) {
    print(p_temps)
    print(p_temps_scaled)
  }

  # --- Graphique pour les erreurs de précision (si disponibles) : SANS filtrage ---
  if (donnees_list$has_precision) {
    donnees_precision <- donnees %>% filter(!is.na(Precision_Error_Avg_px))
    p_precision <- ggplot(donnees_precision, aes(x = Precision_Error_Avg_px, fill = Parser_Type, color = Parser_Type)) +
      geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
      geom_density(alpha = 0.6) +
      facet_wrap(~ Parser_Type, scales = "free") +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      scale_color_manual(values = COULEURS_PARSEURS) +
      labs(title = "Distribution des erreurs de précision par parseur",
           subtitle = "Distribution réelle (aucun filtrage)",
           x = "Erreur de précision (pixels)",
           y = "Densité") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none",
        strip.text = element_text(face = "bold", size = 12)
      )

    # Version scaled (axe limité, outliers masqués)
    stats_par_parseur_prec <- donnees_precision %>%
      group_by(Parser_Type) %>%
      summarise(
        Q1 = quantile(Precision_Error_Avg_px, 0.25, na.rm = TRUE),
        Q3 = quantile(Precision_Error_Avg_px, 0.75, na.rm = TRUE),
        IQR = Q3 - Q1,
        .groups = "drop"
      )
    max_y_prec <- max(stats_par_parseur_prec$Q3 + 1.5 * stats_par_parseur_prec$IQR, na.rm = TRUE)
    p_precision_scaled <- ggplot(donnees_precision, aes(x = Precision_Error_Avg_px, fill = Parser_Type, color = Parser_Type)) +
      geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
      geom_density(alpha = 0.6) +
      facet_wrap(~ Parser_Type, scales = "free") +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      scale_color_manual(values = COULEURS_PARSEURS) +
      coord_cartesian(xlim = c(0, max_y_prec)) +
      labs(title = "Distribution des erreurs de précision par parseur (échelle limitée)",
           subtitle = "Distribution réelle (axe limité, outliers masqués)",
           x = "Erreur de précision (pixels)",
           y = "Densité") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none",
        strip.text = element_text(face = "bold", size = 12)
      )

    ggsave("analysis_results/inter_parseurs/distribution_gaussienne_precision.png", p_precision, width = 12, height = 8, bg = "white")
    ggsave("analysis_results/inter_parseurs/distribution_gaussienne_precision_scaled.png", p_precision_scaled, width = 12, height = 8, bg = "white")

    if (interactive()) {
      print(p_precision)
      print(p_precision_scaled)
    }

    return(list(temps = p_temps, temps_scaled = p_temps_scaled, precision = p_precision, precision_scaled = p_precision_scaled))
  }

  return(list(temps = p_temps, temps_scaled = p_temps_scaled))
}

# 11. Fonction d'analyse principale qui exécute toutes les analyses
analyse_comparative_inter_parseurs <- function(chemin_csv) {
  # Créer le dossier pour les résultats
  creer_dossier_inter_parseurs()
  
  # Charger les données
  donnees_list <- charger_donnees(chemin_csv)
  
  # Exécuter toutes les analyses pertinentes
  resultats <- list(
    boxplot_temps = graphique_boxplot_temps(donnees_list),
    barres_performance = graphique_barres_performance(donnees_list),
    evolution_performances = graphique_evolution_performances(donnees_list),
    scatter_temps_precision = graphique_scatter_temps_precision(donnees_list),
    violin_temps = graphique_violin_temps(donnees_list),
    front_pareto_global = graphique_pareto_temps_global(donnees_list, lisser = FALSE, scaled = FALSE),
    front_pareto_global_scaled = graphique_pareto_temps_global(donnees_list, lisser = FALSE, scaled = TRUE),
    erreurs_coins = graphique_erreurs_coins(donnees_list),
    tableau_performances = generer_tableau_performances(donnees_list),
    radar_performance = graphique_radar_performance(donnees_list),
    perf_par_config = graphique_perf_par_config(donnees_list),
    courbes_gaussiennes = graphique_courbes_gaussiennes(donnees_list)
  )
  

  # Retourner la liste des résultats
  return(resultats)
}



# 13. Fonction principale pour exécuter l'analyse
main <- function() {
  cat("=== Script d'Analyse Comparative Inter-Parseurs ===\n")
  
  # Si lancé en mode interactif, demander le chemin du fichier CSV
  if (interactive()) {
    chemin_csv <- readline(prompt = "Entrez le chemin du fichier CSV contenant les données (ou appuyez sur Entrée pour utiliser le chemin par défaut): ")
    if (chemin_csv == "") {
      chemin_csv <- "results.csv" # Chemin par défaut
    }
  } else {
    # En mode non-interactif, utiliser le premier argument de ligne de commande ou le chemin par défaut
    args <- commandArgs(trailingOnly = TRUE)
    if (length(args) > 0) {
      chemin_csv <- args[1]
    } else {
      chemin_csv <- "results.csv" # Chemin par défaut
    }
  }
  
  # Lancer l'analyse comparative
  resultats <- analyse_comparative_inter_parseurs(chemin_csv)
  
  cat("\n=== Analyse comparative inter-parseurs terminée avec succès! ===\n")
  cat("Les résultats ont été enregistrés dans le dossier 'analysis_results/inter_parseurs'\n")
  
  return(invisible(resultats))
}

# Exécuter la fonction principale si le script est lancé directement
if (!interactive() || (exists("run_analysis") && run_analysis)) {
  main()
}