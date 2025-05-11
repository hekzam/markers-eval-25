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
  
  # Créer le graphique à moustaches
  p <- ggplot(donnees, aes(x = Parser_Type, y = Parsing_Time_ms, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Comparaison des temps d'exécution entre parseurs",
         subtitle = "Distribution des temps de parsing pour chaque parseur",
         x = "Type de parseur",
         y = "Temps de parsing (ms)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none"  # Masquer la légende car les couleurs sont redondantes avec l'axe x
    )
  
  # Si les données de configuration existent, créer aussi des boxplots par configuration
  if (donnees_list$has_config) {
    # Vérifier le nombre de configurations pour éviter les graphiques trop chargés
    nb_configs <- length(unique(donnees$Copy_Config))
    
    if (nb_configs <= 5) {  # Limite arbitraire pour la lisibilité
      p_config <- ggplot(donnees, aes(x = Parser_Type, y = Parsing_Time_ms, fill = Parser_Type)) +
        geom_boxplot(alpha = 0.7) +
        facet_wrap(~ Copy_Config, scales = "free_y") +
        scale_fill_manual(values = COULEURS_PARSEURS) +
        labs(title = "Comparaison des temps d'exécution entre parseurs par configuration",
             subtitle = "Distribution des temps de parsing par type de configuration",
             x = "Type de parseur",
             y = "Temps de parsing (ms)") +
        theme_minimal() +
        theme(
          plot.title = element_text(face = "bold", size = 14, color = "navy"),
          axis.title = element_text(face = "bold", color = "darkblue"),
          axis.text.x = element_text(angle = 45, hjust = 1),
          legend.position = "none"
        )
      
      # Enregistrer le graphique des temps par configuration
      ggsave("analysis_results/inter_parseurs/boxplot_temps_par_configuration.png", p_config, width = 12, height = 8, bg = "white")
    }
  }
  
  # Afficher le graphique principal si en mode interactif
  if (interactive()) {
    print(p)
  }
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/boxplot_temps_execution.png", p, width = 10, height = 6, bg = "white")
  
  return(p)
}

# 2. Graphique à barres groupées pour comparer la précision ou le taux de succès
graphique_barres_performance <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Préparer une liste pour stocker les graphiques créés
  graphiques <- list()
  
  # Si les données de précision existent, créer un graphique de précision moyenne
  if (donnees_list$has_precision) {
    # Calculer la précision moyenne par parseur
    precision_parseurs <- donnees %>%
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
        )
      
      # Ajouter à la liste de graphiques
      graphiques$succes_config <- p_succes_config
      
      # Enregistrer le graphique
      ggsave("analysis_results/inter_parseurs/barres_taux_succes_par_config.png", p_succes_config, width = 12, height = 7, bg = "white")
      
      if (interactive()) {
        print(p_succes_config)
      }
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
    
    # Calculer les performances moyennes par configuration et parseur
    perf_config <- donnees %>%
      group_by(Parser_Type, Copy_Config, Config_Num) %>%
      summarise(
        Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
        .groups = "drop"
      ) %>%
      arrange(Parser_Type, Config_Num)
    
    # Créer le graphique linéaire d'évolution du temps
    p_temps <- ggplot(perf_config, aes(x = Config_Num, y = Temps_Moyen, color = Parser_Type, group = Parser_Type)) +
      geom_line(size = 1) +
      geom_point(size = 3) +
      scale_color_manual(values = COULEURS_PARSEURS) +
      labs(title = "Évolution du temps d'exécution selon la configuration",
           subtitle = "Comparaison entre parseurs",
           x = "Configuration (croissante)",
           y = "Temps moyen de parsing (ms)",
           color = "Parseur") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "right"
      )
    
    # Enregistrer le graphique
    ggsave("analysis_results/inter_parseurs/evolution_temps_par_config.png", p_temps, width = 12, height = 6, bg = "white")
    
    if (interactive()) {
      print(p_temps)
    }
    
    # Si les données de précision existent, faire aussi un graphique d'évolution de la précision
    if (donnees_list$has_precision) {
      precision_config <- donnees %>%
        group_by(Parser_Type, Copy_Config, Config_Num) %>%
        summarise(
          Erreur_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
          .groups = "drop"
        ) %>%
        arrange(Parser_Type, Config_Num)
      
      p_precision <- ggplot(precision_config, aes(x = Config_Num, y = Erreur_Moyenne, color = Parser_Type, group = Parser_Type)) +
        geom_line(size = 1) +
        geom_point(size = 3) +
        scale_color_manual(values = COULEURS_PARSEURS) +
        labs(title = "Évolution de l'erreur de précision selon la configuration",
             subtitle = "Comparaison entre parseurs",
             x = "Configuration (croissante)",
             y = "Erreur moyenne de précision (pixels)",
             color = "Parseur") +
        theme_minimal() +
        theme(
          plot.title = element_text(face = "bold", size = 14, color = "navy"),
          axis.title = element_text(face = "bold", color = "darkblue"),
          legend.title = element_text(face = "bold", color = "darkblue"),
          legend.position = "right"
        )
      
      # Enregistrer le graphique
      ggsave("analysis_results/inter_parseurs/evolution_precision_par_config.png", p_precision, width = 12, height = 6, bg = "white")
      
      if (interactive()) {
        print(p_precision)
      }
      
      return(list(temps = p_temps, precision = p_precision))
    }
    
    return(list(temps = p_temps))
  }
  
  # Si pas de configuration, retourner NULL
  return(NULL)
}

# 4. Scatter plot pour explorer la relation entre temps et précision
graphique_scatter_temps_precision <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Vérifier si les deux colonnes nécessaires existent
  if (!donnees_list$has_precision) {
    cat("Scatter plot temps vs précision non disponible : données de précision manquantes\n")
    return(NULL)
  }
  
  # Filtrer les valeurs aberrantes
  # Calcul des statistiques pour déterminer les valeurs aberrantes
  Q1_temps <- quantile(donnees$Parsing_Time_ms, 0.25, na.rm = TRUE)
  Q3_temps <- quantile(donnees$Parsing_Time_ms, 0.75, na.rm = TRUE)
  IQR_temps <- Q3_temps - Q1_temps
  
  Q1_precision <- quantile(donnees$Precision_Error_Avg_px, 0.25, na.rm = TRUE)
  Q3_precision <- quantile(donnees$Precision_Error_Avg_px, 0.75, na.rm = TRUE)
  IQR_precision <- Q3_precision - Q1_precision
  
  # Définir les seuils pour les valeurs aberrantes (1.5 × IQR est un seuil standard)
  seuil_inf_temps <- Q1_temps - 1.5 * IQR_temps
  seuil_sup_temps <- Q3_temps + 1.5 * IQR_temps
  
  seuil_inf_precision <- Q1_precision - 1.5 * IQR_precision
  seuil_sup_precision <- Q3_precision + 1.5 * IQR_precision
  
  # Filtrer les données
  donnees_filtrees <- donnees %>%
    filter(
      Parsing_Time_ms >= seuil_inf_temps & 
      Parsing_Time_ms <= seuil_sup_temps &
      Precision_Error_Avg_px >= seuil_inf_precision & 
      Precision_Error_Avg_px <= seuil_sup_precision
    )
  
  # Ajouter un message indiquant combien de points ont été filtrés
  nb_points_filtres <- nrow(donnees) - nrow(donnees_filtrees)
  cat(paste0("Scatter plot temps vs précision : ", nb_points_filtres, " points aberrants filtrés (", 
             round(nb_points_filtres / nrow(donnees) * 100, 1), "% des données)\n"))
  
  # Créer le scatter plot avec les données filtrées
  p <- ggplot(donnees_filtrees, aes(x = Parsing_Time_ms, y = Precision_Error_Avg_px, color = Parser_Type)) +
    geom_point(size = 3, alpha = 0.7) +
    scale_color_manual(values = COULEURS_PARSEURS) +
    labs(title = "Relation entre temps d'exécution et erreur de précision",
         subtitle = paste0("Données filtrées sans valeurs aberrantes (", nb_points_filtres, " points exclus)"),
         x = "Temps de parsing (ms)",
         y = "Erreur de précision moyenne (pixels)",
         color = "Parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right"
    )
  
  # Ajouter des moyennes par parseur (calculées sur les données filtrées)
  moyennes_parseurs <- donnees_filtrees %>%
    group_by(Parser_Type) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      Erreur_Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
      .groups = "drop"
    )
  
  p <- p + 
    geom_point(data = moyennes_parseurs, 
              aes(x = Temps_Moyen, y = Erreur_Moyenne, color = Parser_Type),
              size = 5, shape = 18) +
    geom_text(data = moyennes_parseurs,
              aes(x = Temps_Moyen, y = Erreur_Moyenne, label = Parser_Type),
              vjust = -1, hjust = 0.5, size = 3.5, fontface = "bold")
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/scatter_temps_vs_precision.png", p, width = 10, height = 8, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
  return(p)
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
    
    # Créer le violin plot avec les données filtrées
    p2 <- ggplot(donnees_filtrees, aes(x = Parser_Type, y = Precision_Error_Avg_px, fill = Parser_Type)) +
      geom_violin(trim = FALSE, alpha = 0.7) +
      geom_boxplot(width = 0.1, fill = "white", alpha = 0.5) +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      labs(title = "Distribution des erreurs de précision par parseur",
           subtitle = paste0("Visualisation après filtrage de ", pourcentage_filtre, "% des valeurs aberrantes"),
           x = "Type de parseur",
           y = "Erreur de précision (pixels)") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none"
      )
    
    # Enregistrer le graphique filtré
    ggsave("analysis_results/inter_parseurs/violin_erreur_precision.png", p2, width = 10, height = 6, bg = "white")
    
    # Créer aussi une version avec ylim pour montrer l'ensemble des données mais avec une limite visuelle
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

# 6. Pareto des temps d'exécution moyens par parseur
graphique_pareto_temps <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Calculer les temps moyens par parseur
  temps_moyens <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      .groups = "drop"
    ) %>%
    arrange(desc(Temps_Moyen))  # Trier par temps décroissant
  
  # Calculer le pourcentage cumulatif
  temps_moyens$Pourcentage <- temps_moyens$Temps_Moyen / sum(temps_moyens$Temps_Moyen) * 100
  temps_moyens$Pourcentage_Cumulatif <- cumsum(temps_moyens$Pourcentage)
  
  # Réordonner les parseurs pour le graphique
  temps_moyens$Parser_Type <- factor(temps_moyens$Parser_Type, levels = temps_moyens$Parser_Type)
  
  # Créer le graphique de Pareto
  p <- ggplot(temps_moyens) +
    geom_bar(aes(x = Parser_Type, y = Temps_Moyen, fill = Parser_Type), stat = "identity") +
    geom_text(aes(x = Parser_Type, y = Temps_Moyen, label = round(Temps_Moyen, 2)), vjust = -0.5) +
    geom_line(aes(x = as.integer(Parser_Type), y = Pourcentage_Cumulatif * max(Temps_Moyen) / 100, group = 1), 
             color = "black", size = 1.2) +
    geom_point(aes(x = as.integer(Parser_Type), y = Pourcentage_Cumulatif * max(Temps_Moyen) / 100), 
              color = "black", size = 3) +
    scale_y_continuous(
      name = "Temps moyen de parsing (ms)",
      sec.axis = sec_axis(~ . * 100 / max(temps_moyens$Temps_Moyen), name = "Pourcentage cumulatif (%)")
    ) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Pareto des temps moyens d'exécution par parseur",
         subtitle = "Identifie les parseurs qui contribuent le plus au temps total",
         x = "Type de parseur") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.title.y.right = element_text(color = "darkred", face = "bold"),
      axis.text.y.right = element_text(color = "darkred"),
      legend.position = "none"
    )
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/pareto_temps_moyens.png", p, width = 10, height = 6, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
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
  
  # Créer le graphique avec données filtrées
  p <- ggplot(donnees_coins_filtrees, aes(x = Coin, y = Erreur, fill = Parser_Type)) +
    geom_boxplot(alpha = 0.7) +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    labs(title = "Comparaison des erreurs de précision par coin entre parseurs",
         subtitle = paste0("Distribution des erreurs après filtrage de ", pourcentage_filtre, "% des valeurs aberrantes"),
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
  moyennes_coins <- donnees_coins %>%
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

# 9. Graphique en radar pour comparer les performances globales
graphique_radar_performance <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # Vérifier si les données de précision existent
  if (!donnees_list$has_precision) {
    cat("Graphique radar non disponible : données de précision manquantes\n")
    return(NULL)
  }
  
  # Calculer les métriques normalisées par parseur
  metriques <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Temps_Moyen = mean(Parsing_Time_ms, na.rm = TRUE),
      Erreur_Precision = mean(Precision_Error_Avg_px, na.rm = TRUE),
      .groups = "drop"
    )
  
  # Si données de succès disponibles, les ajouter
  if (donnees_list$has_success) {
    succes <- donnees %>%
      group_by(Parser_Type) %>%
      summarise(
        Taux_Succes = sum(Parsing_Success == TRUE, na.rm = TRUE) / n() * 100,
        .groups = "drop"
      )
    
    metriques <- left_join(metriques, succes, by = "Parser_Type")
  } else {
    # Ajouter une colonne fictive pour éviter les erreurs
    metriques$Taux_Succes <- 100  # Valeur par défaut
  }
  
  # Normaliser les métriques (inverser pour le temps et l'erreur car plus petit = meilleur)
  metriques$Temps_Norm <- 1 - (metriques$Temps_Moyen - min(metriques$Temps_Moyen)) / 
                          (max(metriques$Temps_Moyen) - min(metriques$Temps_Moyen))
  
  metriques$Precision_Norm <- 1 - (metriques$Erreur_Precision - min(metriques$Erreur_Precision)) / 
                             (max(metriques$Erreur_Precision) - min(metriques$Erreur_Precision))
  
  metriques$Succes_Norm <- metriques$Taux_Succes / 100
  
  # Transformer les données pour le format long
  radar_data <- metriques %>%
    select(Parser_Type, Temps_Norm, Precision_Norm, Succes_Norm) %>%
    pivot_longer(cols = c(Temps_Norm, Precision_Norm, Succes_Norm),
                 names_to = "Metrique", values_to = "Valeur")
  
  # Définir l'ordre des métriques pour le graphique radar
  radar_data$Metrique <- factor(radar_data$Metrique, 
                                levels = c("Temps_Norm", "Precision_Norm", "Succes_Norm"),
                                labels = c("Rapidité", "Précision", "Fiabilité"))
  
  # Créer une grille de coordonnées pour le graphique radar
  # Définir les métriques comme axes
  axes <- levels(radar_data$Metrique)
  n_axes <- length(axes)
  
  # Calculer les angles pour chaque axe (en radians)
  angles <- seq(0, 2 * pi, length.out = n_axes + 1)
  angles <- angles[-(n_axes + 1)]  # Retirer le dernier pour éviter la duplication
  
  # Créer un dataframe avec les coordonnées des axes
  axes_df <- data.frame(
    Metrique = axes,
    angle = angles,
    hjust = ifelse(angles > pi, 1, 0),
    vjust = ifelse(angles > pi/2 & angles < 3*pi/2, 1, 0)
  )
  
  # Joindre les angles aux données
  radar_data <- radar_data %>%
    left_join(axes_df, by = "Metrique")
  
  # Calculer les coordonnées x et y pour chaque point
  radar_data$x <- radar_data$Valeur * cos(radar_data$angle)
  radar_data$y <- radar_data$Valeur * sin(radar_data$angle)
  
  # Créer le graphique radar
  p <- ggplot() +
    # Ajouter des lignes de référence circulaires
    geom_polygon(data = data.frame(x = 0.25 * cos(seq(0, 2*pi, length.out = 100)),
                                  y = 0.25 * sin(seq(0, 2*pi, length.out = 100))),
                aes(x, y), fill = NA, color = "gray80") +
    geom_polygon(data = data.frame(x = 0.5 * cos(seq(0, 2*pi, length.out = 100)),
                                  y = 0.5 * sin(seq(0, 2*pi, length.out = 100))),
                aes(x, y), fill = NA, color = "gray80") +
    geom_polygon(data = data.frame(x = 0.75 * cos(seq(0, 2*pi, length.out = 100)),
                                  y = 0.75 * sin(seq(0, 2*pi, length.out = 100))),
                aes(x, y), fill = NA, color = "gray80") +
    geom_polygon(data = data.frame(x = 1.0 * cos(seq(0, 2*pi, length.out = 100)),
                                  y = 1.0 * sin(seq(0, 2*pi, length.out = 100))),
                aes(x, y), fill = NA, color = "gray80") +
    
    # Ajouter les lignes des axes
    geom_segment(data = axes_df, aes(x = 0, y = 0, xend = cos(angle), yend = sin(angle)),
                color = "gray50", linewidth = 0.5) +
    
    # Étiquettes des axes
    geom_text(data = axes_df, 
              aes(x = 1.1 * cos(angle), y = 1.1 * sin(angle), label = Metrique, 
                  hjust = hjust, vjust = vjust),
              color = "darkblue", fontface = "bold", size = 4) +
    
    # Tracer les polygones pour chaque parseur
    geom_polygon(data = radar_data %>% group_by(Parser_Type) %>% arrange(angle),
                aes(x = x, y = y, group = Parser_Type, fill = Parser_Type, color = Parser_Type),
                alpha = 0.2, linewidth = 1) +
    
    # Ajouter les points
    geom_point(data = radar_data, 
              aes(x = x, y = y, color = Parser_Type),
              size = 3) +
    
    # Couleurs et échelles
    scale_fill_manual(values = COULEURS_PARSEURS) +
    scale_color_manual(values = COULEURS_PARSEURS) +
    
    # Titre et légende
    labs(title = "Comparaison des performances des parseurs",
         subtitle = "Les valeurs plus élevées sont meilleures pour toutes les métriques",
         fill = "Parseur",
         color = "Parseur") +
    
    # Ajuster les limites et enlever les axes standards
    coord_equal() +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      legend.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "right",
      axis.text = element_blank(),
      axis.title = element_blank(),
      axis.ticks = element_blank(),
      panel.grid = element_blank()
    )
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/radar_performances.png", p, width = 10, height = 8, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
  return(p)
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
    precision_config <- donnees %>%
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
    )
  
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
      )
    
    # Enregistrer le graphique de précision
    ggsave("analysis_results/inter_parseurs/barres_precision_par_config.png", p_precision, width = 12, height = 7, bg = "white")
    
    graphiques$precision <- p_precision
  }
  
 
  
  if (interactive()) {
    for (g in graphiques) {
      print(g)
    }
  }
  
  return(graphiques)
}

# 7. Graphique avec courbes gaussiennes pour la distribution des temps et erreurs
graphique_courbes_gaussiennes <- function(donnees_list) {
  donnees <- donnees_list$donnees
  
  # --- Filtrage et graphique pour les temps d'exécution ---
  
  # Calculer les statistiques pour filtrer les valeurs aberrantes par parseur
  temps_stats <- donnees %>%
    group_by(Parser_Type) %>%
    summarise(
      Q1 = quantile(Parsing_Time_ms, 0.25, na.rm = TRUE),
      Q3 = quantile(Parsing_Time_ms, 0.75, na.rm = TRUE),
      IQR = Q3 - Q1,
      Seuil_Inf = max(0, Q1 - 1.5 * IQR),  # Pas de temps négatifs
      Seuil_Sup = Q3 + 1.5 * IQR,
      Moyenne = mean(Parsing_Time_ms, na.rm = TRUE),
      Ecart_Type = sd(Parsing_Time_ms, na.rm = TRUE),
      .groups = "drop"
    )
  
  # Filtrer les données par parseur en utilisant les seuils calculés
  donnees_filtrees <- donnees %>%
    left_join(temps_stats, by = "Parser_Type") %>%
    filter(Parsing_Time_ms >= Seuil_Inf & Parsing_Time_ms <= Seuil_Sup)
  
  # Calculer le pourcentage de données filtrées
  pct_filtre <- (1 - nrow(donnees_filtrees) / nrow(donnees)) * 100
  cat(sprintf("Courbes gaussiennes de temps : %.1f%% des valeurs filtrées comme aberrantes\n", pct_filtre))
  
  # Créer les paramètres pour les courbes gaussiennes théoriques par parseur
  params_gaussian <- donnees_filtrees %>%
    group_by(Parser_Type) %>%
    summarise(
      Moyenne = mean(Parsing_Time_ms, na.rm = TRUE),
      Ecart_Type = sd(Parsing_Time_ms, na.rm = TRUE),
      .groups = "drop"
    )
  
  # Créer le graphique de densité avec courbe gaussienne superposée
  p_temps <- ggplot(donnees_filtrees, aes(x = Parsing_Time_ms, fill = Parser_Type, color = Parser_Type)) +
    # Histogramme en transparence pour voir la distribution réelle
    geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
    # Courbe de densité pour montrer la distribution réelle
    geom_density(alpha = 0.6) +
    # Ajouter une courbe gaussienne théorique pour chaque parseur
    stat_function(data = params_gaussian, 
                 aes(x = NULL, y = NULL, color = Parser_Type),
                 fun = function(x, mean, sd) dnorm(x, mean, sd),
                 args = list(
                   mean = params_gaussian$Moyenne,
                   sd = params_gaussian$Ecart_Type
                 ),
                 size = 1, linetype = "dashed") +
    facet_wrap(~ Parser_Type, scales = "free") +
    scale_fill_manual(values = COULEURS_PARSEURS) +
    scale_color_manual(values = COULEURS_PARSEURS) +
    labs(title = "Distribution des temps d'exécution par parseur avec courbes gaussiennes",
         subtitle = sprintf("Distribution réelle (ligne pleine) vs gaussienne théorique (pointillée) - %.1f%% des valeurs aberrantes filtrées", pct_filtre),
         x = "Temps de parsing (ms)",
         y = "Densité") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      legend.position = "none",
      strip.text = element_text(face = "bold", size = 12)
    )
  
  # Enregistrer le graphique
  ggsave("analysis_results/inter_parseurs/distribution_gaussienne_temps.png", p_temps, width = 12, height = 8, bg = "white")
  
  if (interactive()) {
    print(p_temps)
  }
  
  # --- Filtrage et graphique pour les erreurs de précision (si disponibles) ---
  if (donnees_list$has_precision) {
    # Calculer les statistiques pour filtrer les valeurs aberrantes par parseur
    precision_stats <- donnees %>%
      filter(!is.na(Precision_Error_Avg_px)) %>%
      group_by(Parser_Type) %>%
      summarise(
        Q1 = quantile(Precision_Error_Avg_px, 0.25, na.rm = TRUE),
        Q3 = quantile(Precision_Error_Avg_px, 0.75, na.rm = TRUE),
        IQR = Q3 - Q1,
        Seuil_Inf = max(0, Q1 - 1.5 * IQR),  # Pas d'erreurs négatives
        Seuil_Sup = Q3 + 1.5 * IQR,
        Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
        Ecart_Type = sd(Precision_Error_Avg_px, na.rm = TRUE),
        .groups = "drop"
      )
    
    # Filtrer les données par parseur
    donnees_precision_filtrees <- donnees %>%
      filter(!is.na(Precision_Error_Avg_px)) %>%
      left_join(precision_stats, by = "Parser_Type") %>%
      filter(Precision_Error_Avg_px >= Seuil_Inf & Precision_Error_Avg_px <= Seuil_Sup)
    
    # Calculer le pourcentage de données filtrées
    pct_filtre_precision <- (1 - nrow(donnees_precision_filtrees) / sum(!is.na(donnees$Precision_Error_Avg_px))) * 100
    cat(sprintf("Courbes gaussiennes de précision : %.1f%% des valeurs filtrées comme aberrantes\n", pct_filtre_precision))
    
    # Calculer les paramètres des gaussiennes théoriques
    params_precision <- donnees_precision_filtrees %>%
      group_by(Parser_Type) %>%
      summarise(
        Moyenne = mean(Precision_Error_Avg_px, na.rm = TRUE),
        Ecart_Type = sd(Precision_Error_Avg_px, na.rm = TRUE),
        .groups = "drop"
      )
    
    # Créer le graphique pour la précision
    p_precision <- ggplot(donnees_precision_filtrees, aes(x = Precision_Error_Avg_px, fill = Parser_Type, color = Parser_Type)) +
      geom_histogram(aes(y = ..density..), alpha = 0.3, position = "identity", bins = 30) +
      geom_density(alpha = 0.6) +
      stat_function(data = params_precision,
                   aes(x = NULL, y = NULL, color = Parser_Type),
                   fun = function(x, mean, sd) dnorm(x, mean, sd),
                   args = list(
                     mean = params_precision$Moyenne,
                     sd = params_precision$Ecart_Type
                   ),
                   size = 1, linetype = "dashed") +
      facet_wrap(~ Parser_Type, scales = "free") +
      scale_fill_manual(values = COULEURS_PARSEURS) +
      scale_color_manual(values = COULEURS_PARSEURS) +
      labs(title = "Distribution des erreurs de précision par parseur avec courbes gaussiennes",
           subtitle = sprintf("Distribution réelle (ligne pleine) vs gaussienne théorique (pointillée) - %.1f%% des valeurs aberrantes filtrées", pct_filtre_precision),
           x = "Erreur de précision (pixels)",
           y = "Densité") +
      theme_minimal() +
      theme(
        plot.title = element_text(face = "bold", size = 14, color = "navy"),
        axis.title = element_text(face = "bold", color = "darkblue"),
        legend.position = "none",
        strip.text = element_text(face = "bold", size = 12)
      )
    
    # Enregistrer le graphique
    ggsave("analysis_results/inter_parseurs/distribution_gaussienne_precision.png", p_precision, width = 12, height = 8, bg = "white")
    
    if (interactive()) {
      print(p_precision)
    }
    
    return(list(temps = p_temps, precision = p_precision))
  }
  
  return(list(temps = p_temps))
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
    pareto_temps = graphique_pareto_temps(donnees_list),
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