# =================================================================
# ANALYSE DE L'IMPACT DU BRUIT SUR LE PARSING
# Script R pour générer 10 graphiques d'analyse
# =================================================================

# Chargement des bibliothèques nécessaires
library(ggplot2)
library(dplyr)
library(tidyr)
library(gridExtra)
library(viridis)
library(scales)
library(corrplot)

# =================================================================
# 1. CHARGEMENT ET PRÉPARATION DES DONNÉES
# =================================================================

# Chargement du fichier CSV
data <- read.csv("../output/csv/limite.csv", stringsAsFactors = FALSE)

# NE PAS convertir Value_Mofication en numérique ici !
# data$Value_Mofication <- as.numeric(as.character(data$Value_Mofication))

# Suppression du filtrage des types de transformation
# types_a_exclure <- c("dpi", "encoded_marker_size", "grey_level", "stroke_width", "unencoded_marker_size")
# data <- data %>% filter(!Modification_Type %in% types_a_exclure)

# Affichage de la structure des données
cat("Structure des données:\n")
str(data)
cat("\nPremières lignes:\n")
head(data)

# Vérification des colonnes disponibles
cat("\nColonnes disponibles:\n")
print(colnames(data))

# Calcul du taux de succès par groupe
success_rate <- data %>%
  group_by(Modification_Type, Value_Mofication) %>%
  summarise(
    Success_Rate = mean(Parsing_Success, na.rm = TRUE),
    .groups = 'drop'
  )

# =================================================================
# 2. CONFIGURATION DES THÈMES ET COULEURS
# =================================================================

# Thème personnalisé pour tous les graphiques
# Ajout d'un fond blanc
theme_custom <- theme_minimal(base_family = "") +
  theme(
    plot.background = element_rect(fill = "white", color = NA),
    panel.background = element_rect(fill = "white", color = NA),
    panel.grid.major = element_line(color = "#e5e5e5"),
    panel.grid.minor = element_line(color = "#f5f5f5"),
    plot.title = element_text(size = 14, face = "bold", hjust = 0.5),
    plot.subtitle = element_text(size = 11, hjust = 0.5),
    axis.title = element_text(size = 11),
    axis.text = element_text(size = 10),
    legend.title = element_text(size = 11),
    legend.text = element_text(size = 10),
    strip.text = element_text(size = 10, face = "bold")
  )

# Palette de couleurs
colors_noise <- scale_color_viridis_d(option = "plasma")
fill_noise <- scale_fill_viridis_d(option = "plasma")

# =================================================================
# 7. GRAPHIQUE 5 - BOXPLOT PRECISION ERROR PAR TYPE DE BRUIT
# =================================================================

p5 <- ggplot(data, aes(x = Modification_Type, y = Precision_Error_Avg_px, fill = Modification_Type)) +
  geom_boxplot(alpha = 0.7) +
  geom_jitter(alpha = 0.3, width = 0.2) +
  fill_noise +
  labs(
    title = "5. Distribution des Erreurs de Précision par Type de Bruit",
    subtitle = "Boxplot montrant la variabilité des erreurs",
    x = "Type de Bruit",
    y = "Erreur de Précision Moyenne (px)",
    fill = "Type de Bruit"
  ) +
  theme_custom +
  theme(axis.text.x = element_text(angle = 45, hjust = 1))

# =================================================================
# 7.1. GRAPHIQUE 5 SCALED - BOXPLOT PRECISION ERROR PAR TYPE DE BRUIT (NORMALISÉ)
# =================================================================

# Calcul du z-score de Precision_Error_Avg_px par Modification_Type
data_scaled <- data %>%
  group_by(Modification_Type) %>%
  mutate(
    Precision_Error_Avg_px_scaled = scale(Precision_Error_Avg_px)
  ) %>%
  ungroup()

p5_scaled <- ggplot(data_scaled, aes(x = Modification_Type, y = Precision_Error_Avg_px_scaled, fill = Modification_Type)) +
  geom_boxplot(alpha = 0.7) +
  geom_jitter(alpha = 0.3, width = 0.2) +
  fill_noise +
  labs(
    title = "5b. Distribution des Erreurs de Précision (Normalisée) par Type de Bruit",
    subtitle = "Boxplot (z-score) pour comparer la variabilité relative",
    x = "Type de Bruit",
    y = "Erreur de Précision Moyenne (z-score)",
    fill = "Type de Bruit"
  ) +
  theme_custom +
  theme(axis.text.x = element_text(angle = 45, hjust = 1))

# =================================================================
# 8. GRAPHIQUE 6 - VIOLIN PLOT PARSING TIME PAR TYPE DE BRUIT
# =================================================================

p6 <- ggplot(data, aes(x = Modification_Type, y = Parsing_Time_ms, fill = Modification_Type)) +
  geom_violin(alpha = 0.7) +
  geom_boxplot(width = 0.2, alpha = 0.8) +
  fill_noise +
  labs(
    title = "6. Distribution du Temps de Parsing par Type de Bruit",
    subtitle = "Violin plot montrant la distribution des temps de traitement",
    x = "Type de Bruit",
    y = "Temps de Parsing (ms)",
    fill = "Type de Bruit"
  ) +
  theme_custom +
  theme(axis.text.x = element_text(angle = 45, hjust = 1))

# =================================================================
# 11. GRAPHIQUE 9 - BAR PLOT TAUX DE RÉUSSITE
# =================================================================

success_summary <- data %>%
  group_by(Modification_Type) %>%
  summarise(Success_Rate = mean(Parsing_Success, na.rm = TRUE), .groups = 'drop')

p9 <- ggplot(success_summary, aes(x = Modification_Type, y = Success_Rate, fill = Modification_Type)) +
  geom_col(alpha = 0.8) +
  geom_text(aes(label = paste0(round(Success_Rate*100, 1), "%")), 
            vjust = -0.5, size = 4) +
  fill_noise +
  scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
  labs(
    title = "9. Taux de Réussite Globale par Type de Bruit",
    subtitle = "Robustesse de l'algorithme face aux différents types de bruit",
    x = "Type de Bruit",
    y = "Taux de Succès Moyen (%)",
    fill = "Type de Bruit"
  ) +
  theme_custom +
  theme(axis.text.x = element_text(angle = 45, hjust = 1))

# =================================================================
# GRAPHIQUE A - SUCCÈS PAR TYPE DE BRUIT POUR CHAQUE PARSER (UN FICHIER PAR PARSER)
# =================================================================

# Création d'un répertoire pour sauvegarder les graphiques
plots_dir <- "./limite"
dir.create(plots_dir, recursive = TRUE, showWarnings = FALSE)

if ("Parser_Type" %in% colnames(data)) {
  success_by_parser_noise <- data %>%
    group_by(Parser_Type, Modification_Type) %>%
    summarise(Success_Rate = mean(Parsing_Success, na.rm = TRUE), .groups = 'drop')

  for (parser in unique(success_by_parser_noise$Parser_Type)) {
    df <- success_by_parser_noise %>% filter(Parser_Type == parser)
    pA_ind <- ggplot(df, aes(x = Modification_Type, y = Success_Rate, fill = Modification_Type)) +
      geom_col(alpha = 0.8) +
      fill_noise +
      scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
      labs(
        title = paste0("Succès par Type de Bruit - ", parser),
        x = "Type de Bruit",
        y = "Taux de Succès (%)",
        fill = "Type de Bruit"
      ) +
      theme_custom +
      theme(axis.text.x = element_text(angle = 45, hjust = 1))
    ggsave(file.path(plots_dir, paste0("A_success_by_noise_", parser, ".png")), pA_ind, width = 10, height = 7, dpi = 300)
  }
}

# =================================================================
# GRAPHIQUE B - SUCCÈS PAR VALEUR DE BRUIT ET TYPE DE BRUIT POUR CHAQUE PARSER (UN FICHIER PAR PARSER ET PAR BRUIT)
# =================================================================

if ("Parser_Type" %in% colnames(data)) {
  success_by_parser_noise_value <- data %>%
    group_by(Parser_Type, Modification_Type, Value_Mofication) %>%
    summarise(Success_Rate = mean(Parsing_Success, na.rm = TRUE), .groups = 'drop')

  # === AJOUT : Pour stroke_width, tracer aussi les erreurs de précision par coin ===
  for (parser in unique(success_by_parser_noise_value$Parser_Type)) {
    parser_dir <- file.path(plots_dir, parser)
    dir.create(parser_dir, recursive = TRUE, showWarnings = FALSE)
    for (noise in unique(success_by_parser_noise_value$Modification_Type)) {
      df <- success_by_parser_noise_value %>% filter(Parser_Type == parser, Modification_Type == noise)
      if (nrow(df) > 0) {
        if (noise == "add_gaussian_noise") {
          # Extraction robuste de mu et sigma depuis Value_Mofication
          df <- df %>% mutate(
            mu = as.numeric(sub("^\\(([^|]+)\\s*\\|.*$", "\\1", Value_Mofication)),
            sigma = as.numeric(sub("^\\([^|]+\\s*\\|\\s*([^\\)]+)\\)$", "\\1", Value_Mofication))
          )
          pB_ind <- ggplot(df, aes(x = mu, y = Success_Rate, color = as.factor(sigma), group = sigma)) +
            geom_line(linewidth = 1.2) +
            geom_point(size = 2) +
            scale_color_viridis_d(name = "Sigma") +
            scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
            labs(
              title = paste0("Succès par Valeur de Bruit (mu, sigma) - ", parser, " (", noise, ")"),
              x = "Mu (moyenne du bruit)",
              y = "Taux de Succès (%)"
            ) +
            theme_custom
        } else if (noise == "add_ink_stain") {
          # Extraction des 3 paramètres pour add_ink_stain
          df <- df %>% mutate(
            n_stains = as.numeric(sub("^\\(([^|]+)\\s*\\|.*$", "\\1", Value_Mofication)),
            mean_size = as.numeric(sub("^\\([^|]+\\s*\\|\\s*([^|]+)\\s*\\|.*$", "\\1", Value_Mofication)),
            opacity = as.numeric(sub("^\\([^|]+\\s*\\|\\s*[^|]+\\s*\\|\\s*([^\\)]+)\\)$", "\\1", Value_Mofication))
          )
          pB_ind <- ggplot(df, aes(x = n_stains, y = Success_Rate, color = as.factor(mean_size), shape = as.factor(opacity), group = interaction(mean_size, opacity))) +
            geom_line(linewidth = 1.2) +
            geom_point(size = 2) +
            scale_color_viridis_d(name = "Taille moyenne") +
            scale_shape_discrete(name = "Opacité") +
            scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
            labs(
              title = paste0("Succès par Nombre de Taches d'Encre - ", parser, " (", noise, ")"),
              x = "Nombre de taches d'encre",
              y = "Taux de Succès (%)"
            ) +
            theme_custom
        } else if (noise == "contrast_brightness_modifier") {
          # Extraction des 2 paramètres pour contrast_brightness_modifier
          df <- df %>% mutate(
            contrast = as.numeric(sub("^\\(([^|]+)\\s*\\|.*$", "\\1", Value_Mofication)),
            brightness = as.numeric(sub("^\\([^|]+\\s*\\|\\s*([^\\)]+)\\)$", "\\1", Value_Mofication))
          )
          pB_ind <- ggplot(df, aes(x = contrast, y = Success_Rate, color = as.factor(brightness), group = brightness)) +
            geom_line(linewidth = 1.2) +
            geom_point(size = 2) +
            scale_color_viridis_d(name = "Luminosité") +
            scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
            labs(
              title = paste0("Succès par Contraste - ", parser, " (", noise, ")"),
              x = "Variation du contraste",
              y = "Taux de Succès (%)"
            ) +
            theme_custom
        } else if (noise == "stroke_width") {
          # Nouveau : Graphe simple erreurs de précision par coin pour stroke_width
          data_stroke <- data %>%
            filter(Parser_Type == parser, Modification_Type == "stroke_width") %>%
            mutate(Value_Mofication_num = as.numeric(as.character(Value_Mofication)))
          if (nrow(data_stroke) > 0) {
            data_stroke_long <- data_stroke %>%
              select(Value_Mofication_num,
                     Precision_Error_TopLeft_px,
                     Precision_Error_TopRight_px,
                     Precision_Error_BottomLeft_px,
                     Precision_Error_BottomRight_px) %>%
              tidyr::pivot_longer(
                cols = starts_with("Precision_Error_"),
                names_to = "Corner",
                values_to = "Precision_Error"
              ) %>%
              mutate(
                Corner = recode(Corner,
                  "Precision_Error_TopLeft_px" = "TopLeft",
                  "Precision_Error_TopRight_px" = "TopRight",
                  "Precision_Error_BottomLeft_px" = "BottomLeft",
                  "Precision_Error_BottomRight_px" = "BottomRight"
                )
              )
            # Style simple : scatterplot + lignes lissées, fond blanc
            pB_stroke_corners <- ggplot(data_stroke_long, aes(x = Value_Mofication_num, y = Precision_Error, color = Corner)) +
              geom_point(size = 2, alpha = 0.7) +
              geom_smooth(se = FALSE, method = "loess", linewidth = 1) +
              scale_color_brewer(palette = "Dark2") +
              labs(
                title = paste0("Effet de stroke_width sur l'erreur de précision des coins (", parser, ")"),
                x = "stroke_width",
                y = "Erreur de Précision (px)",
                color = "Coin"
              ) +
              theme_minimal(base_size = 13) +
              theme(
                plot.background = element_rect(fill = "white", color = NA),
                panel.background = element_rect(fill = "white", color = NA),
                plot.title = element_text(face = "bold", hjust = 0.5),
                legend.position = "top",
                panel.grid.minor = element_blank()
              )
            ggsave(file.path(parser_dir, paste0("B_precision_error_by_stroke_width_corners_", parser, ".png")), pB_stroke_corners, width = 8, height = 5, dpi = 300)
          }
          # Graphe succès classique déjà existant :
          pB_ind <- ggplot(df, aes(x = as.numeric(Value_Mofication), y = Success_Rate)) +
            geom_line(linewidth = 1.2, color = "#440154FF") +
            geom_point(size = 2, color = "#440154FF") +
            scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
            labs(
              title = paste0("Succès par Valeur de Bruit - ", parser, " (", noise, ")"),
              x = "stroke_width",
              y = "Taux de Succès (%)"
            ) +
            theme_custom
        } else if (noise == "translate_img") {
          # Extraction des deux paramètres pour translate_img (x, y)
          df <- df %>%
            mutate(
              translate_x = as.numeric(sub("^\\(([^|]+)\\s*\\|.*$", "\\1", Value_Mofication)),
              translate_y = as.numeric(sub("^\\([^|]+\\s*\\|\\s*([^\\)]+)\\)$", "\\1", Value_Mofication))
            )
          # Graphe : scatterplot des points de translation, couleur = taux de succès
          pB_ind <- ggplot(df, aes(x = translate_x, y = translate_y, color = Success_Rate)) +
            geom_point(size = 3, alpha = 0.8) +
            scale_color_viridis_c(option = "plasma", limits = c(0, 1), labels = percent_format(accuracy = 1)) +
            labs(
              title = paste0("Succès par Translation (X, Y) - ", parser, " (", noise, ")"),
              x = "Translation X (px)",
              y = "Translation Y (px)",
              color = "Taux de Succès"
            ) +
            theme_minimal(base_size = 13) +
            theme(
              plot.background = element_rect(fill = "white", color = NA),
              panel.background = element_rect(fill = "white", color = NA),
              plot.title = element_text(face = "bold", hjust = 0.5),
              legend.position = "top"
            )
        } else {
          # Détection spécifique pour les graphes de rotation
          x_label <- if (noise == "rotate_img") "Degrés de rotation" else "Niveau de Bruit"
          pB_ind <- ggplot(df, aes(x = as.numeric(Value_Mofication), y = Success_Rate)) +
            geom_line(linewidth = 1.2, color = "#440154FF") +
            geom_point(size = 2, color = "#440154FF") +
            scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
            labs(
              title = paste0("Succès par Valeur de Bruit - ", parser, " (", noise, ")"),
              x = x_label,
              y = "Taux de Succès (%)"
            ) +
            theme_custom
        }
        ggsave(file.path(parser_dir, paste0("B_success_by_noise_value_", parser, "_", noise, ".png")), pB_ind, width = 10, height = 7, dpi = 300)
      }
    }
  }
}

# =================================================================
# GRAPHIQUE C - SUCCÈS PAR VALEUR DE BRUIT (TOUT BRUIT CONFONDU) POUR CHAQUE PARSER
# =================================================================

if ("Parser_Type" %in% colnames(data)) {
  success_by_parser_value <- data %>%
    group_by(Parser_Type, Value_Mofication) %>%
    summarise(Success_Rate = mean(Parsing_Success, na.rm = TRUE), .groups = 'drop')

  pC <- ggplot(success_by_parser_value, aes(x = Value_Mofication, y = Success_Rate, color = Parser_Type)) +
    geom_line(linewidth = 1.2) +
    colors_noise +
    scale_y_continuous(labels = percent_format(), limits = c(0, 1.1)) +
    labs(
      title = "Succès par Niveau de Bruit (tous types confondus) pour chaque Parser",
      x = "Niveau de Bruit",
      y = "Taux de Succès (%)",
      color = "Type de Parser"
    ) +
    theme_custom
}

# =================================================================
# AFFICHAGE ET SAUVEGARDE DES GRAPHIQUES
# =================================================================

# Affichage de tous les graphiques
# print(p1)
# print(p2)
# print(p3)
# print(p4)  # Suppression de l'affichage du heatmap
print(p5)
print(p5_scaled) # Affichage du boxplot normalisé
print(p6)
print(p9)
# Suppression de l'affichage du dual axis
# print(p10)

# Sauvegarde des graphiques en haute résolution
# ggsave(file.path(plots_dir, "01_parsing_time_vs_noise.png"), p1, width = 12, height = 8, dpi = 300)
# ggsave(file.path(plots_dir, "02_success_rate_vs_noise.png"), p2, width = 12, height = 8, dpi = 300)
# ggsave(file.path(plots_dir, "03_precision_error_vs_noise.png"), p3, width = 12, height = 8, dpi = 300)
# ggsave(file.path(plots_dir, "04_heatmap_errors_by_corner.png"), p4, width = 14, height = 10, dpi = 300)
ggsave(file.path(plots_dir, "05_boxplot_precision_by_noise.png"), p5, width = 12, height = 8, dpi = 300)
ggsave(file.path(plots_dir, "05b_boxplot_precision_by_noise_scaled.png"), p5_scaled, width = 12, height = 8, dpi = 300)
ggsave(file.path(plots_dir, "06_violin_parsing_time.png"), p6, width = 12, height = 8, dpi = 300)
# ggsave(file.path(plots_dir, "07_scatter_time_vs_precision.png"), p7, width = 12, height = 8, dpi = 300)
# ggsave(file.path(plots_dir, "08_multiline_parser_comparison.png"), p8, width = 14, height = 10, dpi = 300)
ggsave(file.path(plots_dir, "09_success_rate_barplot.png"), p9, width = 10, height = 8, dpi = 300)
# Suppression de la sauvegarde du dual axis
# ggsave(file.path(plots_dir, "10_combined_dual_axis.png"), p10, width = 14, height = 10, dpi = 300)

# AFFICHAGE DES NOUVEAUX GRAPHIQUES
if (exists("pA")) print(pA)
if (exists("pB")) print(pB)
if (exists("pC")) print(pC)

# SAUVEGARDE DES NOUVEAUX GRAPHIQUES
if (exists("pA")) ggsave(file.path(plots_dir, "A_success_by_noise_per_parser.png"), pA, width = 12, height = 8, dpi = 300)
if (exists("pB")) ggsave(file.path(plots_dir, "B_success_by_noise_value_per_parser.png"), pB, width = 12, height = 8, dpi = 300)
if (exists("pC")) ggsave(file.path(plots_dir, "C_success_by_noise_value_alltypes_per_parser.png"), pC, width = 12, height = 8, dpi = 300)

# =================================================================
# 14. STATISTIQUES DESCRIPTIVES
# =================================================================

cat("\n=== STATISTIQUES DESCRIPTIVES ===\n")

# Résumé par type de bruit
summary_stats <- data %>%
  group_by(Modification_Type) %>%
  summarise(
    Nb_Observations = n(),
    Temps_Moyen_ms = round(mean(Parsing_Time_ms, na.rm = TRUE), 2),
    Temps_Mediane_ms = round(median(Parsing_Time_ms, na.rm = TRUE), 2),
    Erreur_Moyenne_px = round(mean(Precision_Error_Avg_px, na.rm = TRUE), 2),
    Erreur_Mediane_px = round(median(Precision_Error_Avg_px, na.rm = TRUE), 2),
    Taux_Succes = round(mean(Parsing_Success, na.rm = TRUE) * 100, 1),
    .groups = 'drop'
  )

print(summary_stats)

# Corrélations
cat("\n=== MATRICE DE CORRÉLATION ===\n")
numeric_vars <- data %>% 
  select_if(is.numeric) %>%
  select(-contains("_px")) %>%  # Exclure les colonnes individuelles de coins
  na.omit()

if(ncol(numeric_vars) > 1) {
  correlation_matrix <- cor(numeric_vars)
  print(round(correlation_matrix, 3))
}

cat("\n=== ANALYSE TERMINÉE ===\n")
cat("Tous les graphiques ont été sauvegardés dans './limite/'\n")
cat("Le script a généré 4 visualisations différentes pour analyser l'impact du bruit.\n")

# =================================================================
# GRAPHIQUE STROKE_WIDTH - ERREUR DE PRÉCISION PAR COIN
# =================================================================

# Filtrer les données pour stroke_width
data_stroke <- data %>% filter(Modification_Type == "stroke_width")

if (nrow(data_stroke) > 0) {
  # Conversion de Value_Mofication en numérique si possible
  data_stroke <- data_stroke %>%
    mutate(Value_Mofication_num = as.numeric(as.character(Value_Mofication)))

  # Préparer les données en format long pour ggplot
  data_stroke_long <- data_stroke %>%
    select(Value_Mofication_num,
           Precision_Error_TopLeft_px,
           Precision_Error_TopRight_px,
           Precision_Error_BottomLeft_px,
           Precision_Error_BottomRight_px) %>%
    pivot_longer(
      cols = starts_with("Precision_Error_"),
      names_to = "Corner",
      values_to = "Precision_Error"
    ) %>%
    mutate(
      Corner = recode(Corner,
        "Precision_Error_TopLeft_px" = "TopLeft",
        "Precision_Error_TopRight_px" = "TopRight",
        "Precision_Error_BottomLeft_px" = "BottomLeft",
        "Precision_Error_BottomRight_px" = "BottomRight"
      )
    )

  p_stroke_corners <- ggplot(data_stroke_long, aes(x = Value_Mofication_num, y = Precision_Error, color = Corner)) +
    geom_line(linewidth = 1.2) +
    geom_point(size = 2) +
    scale_color_viridis_d(option = "plasma") +
    labs(
      title = "Erreur de Précision par Coin en fonction de stroke_width",
      x = "stroke_width",
      y = "Erreur de Précision (px)",
      color = "Coin"
    ) +
    theme_custom

  print(p_stroke_corners)
  ggsave(file.path(plots_dir, "B_precision_error_by_stroke_width_corners.png"), p_stroke_corners, width = 10, height = 7, dpi = 300)
}