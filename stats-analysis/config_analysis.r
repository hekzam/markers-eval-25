####################################################
# SCRIPT D'ANALYSE DES CONFIGURATIONS DE MARQUEURS
# Analyse de la consommation d'encre et de l'espace
# occupé par les différentes configurations
####################################################

# Chargement des bibliothèques nécessaires
library(ggplot2)
library(dplyr)
library(tidyr)
library(fs)
library(ggrepel)
library(RColorBrewer)
library(gridExtra)

# Fonction pour créer le dossier de résultats d'analyse de configuration
creer_dossier_config_analyse <- function() {
  dir_create("analysis_results/config_analyse", recurse = TRUE)
  cat("Dossier pour l'analyse des configurations créé avec succès\n")
}

# Fonction pour charger et préparer les données
charger_donnees_config <- function(chemin_csv) {
  # Vérifier si le fichier existe
  if (!file.exists(chemin_csv)) {
    stop("Erreur : Le fichier CSV n'existe pas dans le répertoire indiqué.")
  }
  
  # Lecture du fichier CSV
  donnees <- read.csv(chemin_csv, stringsAsFactors = FALSE)
  
  # Vérifier si les colonnes nécessaires existent
  colonnes_requises <- c("File", "Copy_Config", "Ink_Consumption_ml", "Total_Markers_Area_cm2")
  if (!all(colonnes_requises %in% colnames(donnees))) {
    stop("Erreur : Le fichier CSV ne contient pas les colonnes minimales attendues (File, Copy_Config, Ink_Consumption_ml, Total_Markers_Area_cm2).")
  }
  
  # Ajouter une colonne d'efficacité (rapport surface/encre)
  donnees$Efficiency_Ratio <- donnees$Total_Markers_Area_cm2 / donnees$Ink_Consumption_ml
  
  # Extraire le type principal de marqueur de la configuration
  donnees$Marker_Type <- gsub("([0-9]+\\*)(.*?)( \\| .*|$)", "\\2", donnees$Copy_Config)
  
  # Extraire le nombre de marqueurs du même type
  donnees$Marker_Count <- as.numeric(gsub("([0-9]+)\\*(.*?)( \\| .*|$)", "\\1", donnees$Copy_Config))
  
  # Extraire le type de code (si présent)
  donnees$Code_Type <- ifelse(grepl(" \\| ", donnees$Copy_Config), 
                             gsub(".*? \\| (.*?)( \\| .*|$)", "\\1", donnees$Copy_Config), 
                             NA)
  
  return(donnees)
}

# 1. Graphique à barres pour comparer la consommation d'encre par configuration
graphique_barres_encre <- function(donnees) {
  # Trier les données par consommation d'encre
  donnees_triees <- donnees %>% 
    arrange(Ink_Consumption_ml)
  
  # Créer le graphique à barres
  p <- ggplot(donnees_triees, aes(x = reorder(Copy_Config, Ink_Consumption_ml), y = Ink_Consumption_ml)) +
    geom_bar(stat = "identity", alpha = 0.8, fill = "steelblue") +
    labs(title = "Consommation d'encre par configuration",
         subtitle = "Classement des configurations par consommation d'encre (ml)",
         x = "Configuration",
         y = "Consommation d'encre (ml)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 15, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9, angle = 45, hjust = 1),
      axis.text.y = element_text(face = "bold", size = 9)
    ) +
    coord_flip()
  
  # Enregistrer le graphique
  ggsave("analysis_results/config_analyse/barres_consommation_encre.png", p, width = 12, height = 8, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
  return(p)
}

# 2. Graphique à barres pour comparer la surface totale par configuration
graphique_barres_surface <- function(donnees) {
  # Trier les données par surface
  donnees_triees <- donnees %>% 
    arrange(Total_Markers_Area_cm2)
  
  # Créer le graphique à barres
  p <- ggplot(donnees_triees, aes(x = reorder(Copy_Config, Total_Markers_Area_cm2), y = Total_Markers_Area_cm2)) +
    geom_bar(stat = "identity", alpha = 0.8, fill = "forestgreen") +
    labs(title = "Surface totale occupée par configuration",
         subtitle = "Classement des configurations par surface totale (cm²)",
         x = "Configuration",
         y = "Surface totale (cm²)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9, angle = 45, hjust = 1),
      axis.text.y = element_text(face = "bold", size = 9)
    ) +
    coord_flip()
  
  # Enregistrer le graphique
  ggsave("analysis_results/config_analyse/barres_surface_totale.png", p, width = 12, height = 8, bg = "white")
  
  if (interactive()) {
    print(p)
  }
  
  return(p)
}

# 3. Scatter plot pour explorer la relation entre consommation d'encre et surface
graphique_scatter_encre_surface <- function(donnees) {
  # Créer le scatter plot standard
  p <- ggplot(donnees, aes(x = Ink_Consumption_ml, y = Total_Markers_Area_cm2, color = Copy_Config)) +
    geom_point(size = 4, alpha = 0.8) +
    geom_text_repel(aes(label = Copy_Config), 
                   size = 3.5, 
                   box.padding = 0.5, 
                   point.padding = 0.3,
                   max.overlaps = Inf) + # S'assurer que tous les labels sont affichés
    labs(title = "Relation entre consommation d'encre et surface",
         subtitle = "Scatter plot avec étiquettes des configurations",
         x = "Consommation d'encre (ml)",
         y = "Surface totale (cm²)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 15, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9),
      axis.text.y = element_text(face = "bold", size = 9),
      legend.position = "none"  # Supprimer la légende car les labels sont déjà affichés
    ) +
    expand_limits(x = 0, y = 0) +
    scale_color_brewer(palette = "Dark2") # Utiliser une palette avec des couleurs plus foncées
  
  # Si le nombre de configurations dépasse 8 (limite de la palette Dark2), utiliser une palette plus large
  if(length(unique(donnees$Copy_Config)) > 8) {
    p <- p + scale_color_viridis_d(option = "plasma", begin = 0.2, end = 0.8) # Éviter les couleurs trop claires
  }
  
  # Créer une version zoomée pour améliorer la lisibilité des étiquettes
  # Calculer les limites pour le zoom (50% de la plage des données)
  ink_max <- max(donnees$Ink_Consumption_ml)
  area_max <- max(donnees$Total_Markers_Area_cm2)
  ink_zoom <- ink_max * 0.35  # Zoom à 50% de la plage maximale
  area_zoom <- area_max * 0.35  # Zoom à 50% de la plage maximale
  
  # Filtrer les données pour la vue zoomée
  donnees_zoom <- donnees[donnees$Ink_Consumption_ml <= ink_zoom & donnees$Total_Markers_Area_cm2 <= area_zoom, ]
  
  p_zoom <- ggplot(donnees_zoom, aes(x = Ink_Consumption_ml, y = Total_Markers_Area_cm2, color = Copy_Config)) +
    geom_point(size = 4, alpha = 0.8) +
    geom_text_repel(aes(label = Copy_Config), 
                   size = 4,
                   fontface = "bold",
                   box.padding = 1.3,  # Augmenter l'espace autour du texte
                   point.padding = 0.6,
                   force = 5,  # Plus de force de répulsion
                   max.iter = 5000,  # Plus d'itérations pour le placement
                   max.overlaps = Inf,  # Aucune limite sur les chevauchements
                   min.segment.length = 0,  # Montrer toutes les lignes de connexion
                   segment.size = 0.5) +
    labs(title = "Relation entre consommation d'encre et surface (zoom)",
         subtitle = "Vue rapprochée pour améliorer la lisibilité des étiquettes",
         x = "Consommation d'encre (ml)",
         y = "Surface totale (cm²)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 15, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9),
      axis.text.y = element_text(face = "bold", size = 9),
      plot.margin = margin(0.5, 1, 0.5, 0.5, "cm"),  # Ajouter plus de marge à droite
      legend.position = "none"  # Supprimer la légende car les labels sont déjà affichés
    ) +
    coord_cartesian(xlim = c(0, ink_zoom), ylim = c(0, area_zoom)) +
    scale_color_brewer(palette = "Dark2") # Utiliser une palette avec des couleurs plus foncées
  
  # Si le nombre de configurations zoomées dépasse 8, utiliser une palette plus large
  if(length(unique(donnees_zoom$Copy_Config)) > 8) {
    p_zoom <- p_zoom + scale_color_viridis_d(option = "plasma", begin = 0.2, end = 0.8) # Éviter les couleurs trop claires
  }
  
  # Créer une version encore plus zoomée pour la zone de concentration principale des données
  # Calculer les limites pour le zoom extrême (25% de la plage des données)
  ink_zoom_extreme <- ink_max * 0.25  # Zoom à 25% de la plage maximale
  area_zoom_extreme <- area_max * 0.25  # Zoom à 25% de la plage maximale
  
  # Filtrer les données pour la vue très zoomée
  donnees_zoom_extreme <- donnees[donnees$Ink_Consumption_ml <= ink_zoom_extreme & 
                                 donnees$Total_Markers_Area_cm2 <= area_zoom_extreme, ]
  
  p_zoom_extreme <- ggplot(donnees_zoom_extreme, aes(x = Ink_Consumption_ml, y = Total_Markers_Area_cm2, color = Copy_Config)) +
    geom_point(size = 5, alpha = 0.8) +  # Points légèrement plus gros pour cette vue
    geom_text_repel(aes(label = Copy_Config), 
                   size = 4,  # Texte légèrement plus grand
                   box.padding = 1.2,  
                   point.padding = 0.6,
                   force = 8,  # Force de répulsion accrue
                   max.iter = 8000,  # Plus d'itérations pour le placement
                   max.overlaps = Inf,
                   min.segment.length = 0,
                   segment.size = 0.6) +
    labs(title = "Concentration principale des données (zoom extrême)",
         subtitle = "Vue très rapprochée sur la zone de forte densité des configurations",
         x = "Consommation d'encre (ml)",
         y = "Surface totale (cm²)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 15, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9),
      axis.text.y = element_text(face = "bold", size = 9),
      plot.margin = margin(0.5, 1, 0.5, 0.5, "cm"),
      legend.position = "none"
    ) +
    coord_cartesian(xlim = c(0, ink_zoom_extreme), ylim = c(0, area_zoom_extreme)) +
    scale_color_brewer(palette = "Set1") # Utiliser une palette avec des couleurs vives mais pas trop claires
  
  # Si le nombre de configurations zoomées extrêmes dépasse 9 (limite de Set1), utiliser une palette plus large
  if(length(unique(donnees_zoom_extreme$Copy_Config)) > 9) {
    p_zoom_extreme <- p_zoom_extreme + scale_color_viridis_d(option = "inferno", begin = 0.2, end = 0.8) # Palette inferno: plus foncée que plasma
  }
  
  # Enregistrer les graphiques
  ggsave("analysis_results/config_analyse/scatter_encre_surface.png", p, width = 12, height = 8, bg = "white")
  ggsave("analysis_results/config_analyse/scatter_encre_surface_zoom.png", p_zoom, width = 12, height = 8, bg = "white")
  ggsave("analysis_results/config_analyse/scatter_encre_surface_zoom_extreme.png", p_zoom_extreme, width = 18, height = 12, bg = "white")  # Fichier encore plus grand pour la lisibilité
  
  if (interactive()) {
    print(p)
    print(p_zoom)
    print(p_zoom_extreme)
  }
  
  return(list(standard = p, zoom = p_zoom, zoom_extreme = p_zoom_extreme))
}

# 4. Graphique à barres pour comparer l'efficacité (rapport surface/encre) - FONCTION SUPPRIMÉE

# 5. Front de Pareto pour encre/surface - FONCTION SUPPRIMÉE

# 6. Visualisation de la densité d'information (surface/encre par type de marqueur)
graphique_densite_information <- function(donnees) {
  # Calculer la densité d'information moyenne par type de marqueur
  densite_par_type <- donnees %>%
    group_by(Marker_Type) %>%
    summarise(
      Densité_Moyenne = mean(Efficiency_Ratio, na.rm = TRUE),
      Écart_Type = sd(Efficiency_Ratio, na.rm = TRUE),
      .groups = "drop"
    ) %>%
    arrange(desc(Densité_Moyenne))
  
  # Créer le graphique à barres
  p <- ggplot(densite_par_type, aes(x = reorder(Marker_Type, Densité_Moyenne), y = Densité_Moyenne)) +
    geom_bar(stat = "identity", alpha = 0.8, fill = "orange") +
    geom_errorbar(aes(ymin = pmax(0, Densité_Moyenne - Écart_Type), 
                      ymax = Densité_Moyenne + Écart_Type), 
                  width = 0.2) +
    labs(title = "Densité d'information par type de marqueur",
         subtitle = "Rapport surface/encre moyen (cm²/ml) avec écart-type",
         x = "Type de marqueur",
         y = "Densité d'information (cm²/ml)") +
    theme_minimal() +
    theme(
      plot.title = element_text(face = "bold", size = 14, color = "navy"),
      axis.title = element_text(face = "bold", color = "darkblue"),
      axis.text.x = element_text(face = "bold", size = 9),
      axis.text.y = element_text(face = "bold", size = 9)
    ) +
    coord_flip()
  
  if (interactive()) {
    print(p)
  }
  
  return(p)
}

# 7. Tableau récapitulatif des configurations
generer_tableau_configurations <- function(donnees) {
  # Préparer le tableau récapitulatif
  tableau_config <- donnees %>%
    select(Copy_Config, Marker_Type, Code_Type, Marker_Count, DPI, 
           Ink_Consumption_ml, Total_Markers_Area_cm2, Efficiency_Ratio) %>%
    arrange(desc(Efficiency_Ratio))
  
  # Arrondir les valeurs numériques
  tableau_config$Ink_Consumption_ml <- round(tableau_config$Ink_Consumption_ml, 5)
  tableau_config$Total_Markers_Area_cm2 <- round(tableau_config$Total_Markers_Area_cm2, 2)
  tableau_config$Efficiency_Ratio <- round(tableau_config$Efficiency_Ratio, 2)
  
  # Enregistrer le tableau dans un fichier CSV
  write.csv(tableau_config, "analysis_results/config_analyse/tableau_configurations.csv", row.names = FALSE)
  
  # Afficher le tableau dans la console
  cat("Tableau récapitulatif des configurations :\n")
  print(tableau_config)
  
  return(tableau_config)
}

# 8. Analyse par composant de configuration (nombre de marqueurs, type de code) - FONCTION SUPPRIMÉE

# 9. Fonction d'analyse principale qui exécute toutes les analyses
analyse_config <- function(chemin_csv) {
  # Créer le dossier pour les résultats
  creer_dossier_config_analyse()
  
  # Charger les données
  donnees <- charger_donnees_config(chemin_csv)
  
  # Exécuter toutes les analyses
  resultats <- list(
    barres_encre = graphique_barres_encre(donnees),
    barres_surface = graphique_barres_surface(donnees),
    scatter_encre_surface = graphique_scatter_encre_surface(donnees),
    densite_information = graphique_densite_information(donnees),
    tableau_configurations = generer_tableau_configurations(donnees)
  )
  
  # Retourner la liste des résultats
  return(resultats)
}

# 10. Fonction principale pour exécuter l'analyse
main <- function() {
  cat("=== Script d'Analyse des Configurations de Marqueurs ===\n")
  
  # Si lancé en mode interactif, demander le chemin du fichier CSV
  if (interactive()) {
    chemin_csv <- readline(prompt = "Entrez le chemin du fichier CSV contenant les données (ou appuyez sur Entrée pour utiliser le chemin par défaut): ")
    if (chemin_csv == "") {
      chemin_csv <- "config-analysis.csv" # Chemin par défaut
    }
  } else {
    # En mode non-interactif, utiliser le premier argument de ligne de commande ou le chemin par défaut
    args <- commandArgs(trailingOnly = TRUE)
    if (length(args) > 0) {
      chemin_csv <- args[1]
    } else {
      chemin_csv <- "config-analysis.csv" # Chemin par défaut
    }
  }
  
  # Lancer l'analyse
  resultats <- analyse_config(chemin_csv)
  
  cat("\n=== Analyse des configurations terminée avec succès! ===\n")
  cat("Les résultats ont été enregistrés dans le dossier 'analysis_results/config_analyse'\n")
  
  return(invisible(resultats))
}

# Exécuter la fonction principale si le script est lancé directement
if (!interactive() || (exists("run_analysis") && run_analysis)) {
  main()
}
