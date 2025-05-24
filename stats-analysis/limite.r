# ===========================================
# ANALYSE DES PERFORMANCES DU PARSER
# ===========================================

# Chargement des librairies nécessaires
library(ggplot2)
library(dplyr)
library(tidyr)
library(readr)
library(gridExtra)
library(scales)

# ===========================================
# 1. CHARGEMENT ET PREPARATION DES DONNEES
# ===========================================

# Chargement des données avec gestion d'erreur
tryCatch({
  data <- read_csv("../output/csv/limite.csv", show_col_types = FALSE)
}, error = function(e) {
  stop("Erreur lors du chargement du fichier limite.csv. Vérifiez que le fichier existe.")
})

# Vérification de la structure des données
cat("Structure des données:\n")
str(data)
cat("\nPremières lignes:\n")
head(data)

# Vérification des valeurs manquantes
cat("\nValeurs manquantes par colonne:\n")
missing_values <- sapply(data, function(x) sum(is.na(x)))
print(missing_values)

# Inspection de la colonne Value_Mofication avant conversion
cat("\nValeurs uniques dans Value_Mofication (avant conversion):\n")
print(unique(data$Value_Mofication))

# Nettoyage des données avec gestion robuste
data_clean <- data %>%
  # Conversion sécurisée de Value_Mofication
  mutate(
    Value_Mofication = suppressWarnings(as.numeric(as.character(Value_Mofication))),
    Modification_Type = as.factor(Modification_Type),
    Parser_Type = as.factor(Parser_Type)
  ) %>%
  # Filtrage des lignes valides
  filter(
    !is.na(Precision_Error_Avg_px) & 
    !is.na(Parsing_Time_ms) & 
    !is.na(Value_Mofication) &
    is.finite(Precision_Error_Avg_px) &
    is.finite(Parsing_Time_ms) &
    is.finite(Value_Mofication)
  )

cat("\nNombre de lignes après nettoyage:", nrow(data_clean), "\n")
cat("Nombre de lignes supprimées:", nrow(data) - nrow(data_clean), "\n")

# Vérification qu'il reste des données
if(nrow(data_clean) == 0) {
  stop("Aucune donnée valide après nettoyage. Vérifiez vos données d'entrée.")
}

# ===========================================
# 2. ANALYSE EXPLORATOIRE
# ===========================================

# Statistiques descriptives par type de modification
stats_by_type <- data_clean %>%
  group_by(Modification_Type) %>%
  summarise(
    n = n(),
    precision_mean = mean(Precision_Error_Avg_px, na.rm = TRUE),
    precision_sd = sd(Precision_Error_Avg_px, na.rm = TRUE),
    time_mean = mean(Parsing_Time_ms, na.rm = TRUE),
    time_sd = sd(Parsing_Time_ms, na.rm = TRUE),
    .groups = 'drop'
  )

print("Statistiques par type de modification:")
print(stats_by_type)

# ===========================================
# 3. GRAPHIQUES DE PRECISION MOYENNE
# ===========================================

# Graphique 1: Evolution de la précision moyenne par type de bruit (courbes)
p1 <- data_clean %>%
  group_by(Modification_Type, Value_Mofication) %>%
  summarise(
    mean_precision = mean(Precision_Error_Avg_px, na.rm = TRUE),
    sd_precision = sd(Precision_Error_Avg_px, na.rm = TRUE),
    n = n(),
    se_precision = sd_precision / sqrt(n),
    .groups = 'drop'
  ) %>%
  ggplot(aes(x = Value_Mofication, y = mean_precision, color = Modification_Type)) +
  geom_line(linewidth = 1.2) +
  geom_point(size = 2) +
  geom_errorbar(aes(ymin = mean_precision - se_precision, 
                    ymax = mean_precision + se_precision), 
                width = 0.1, alpha = 0.7) +
  labs(
    title = "Evolution de la Précision Moyenne par Type de Bruit",
    subtitle = "Erreur standard affichée",
    x = "Niveau de Modification",
    y = "Erreur de Précision Moyenne (px)",
    color = "Type de Modification"
  ) +
  theme_minimal() +
  theme(legend.position = "bottom")

# Graphique 2: Boxplots de la précision par niveau de bruit
p2 <- data_clean %>%
  ggplot(aes(x = factor(Value_Mofication), y = Precision_Error_Avg_px, 
             fill = Modification_Type)) +
  geom_boxplot(alpha = 0.7, outlier.alpha = 0.5) +
  facet_wrap(~ Modification_Type, scales = "free") +
  labs(
    title = "Distribution de la Précision par Niveau et Type de Bruit",
    x = "Niveau de Modification",
    y = "Erreur de Précision Moyenne (px)",
    fill = "Type de Modification"
  ) +
  theme_minimal() +
  theme(
    legend.position = "none",
    axis.text.x = element_text(angle = 45, hjust = 1)
  )

# ===========================================
# 4. GRAPHIQUES DE TEMPS DE PARSING
# ===========================================

# Graphique 3: Evolution du temps de parsing par type de bruit (courbes)
p3 <- data_clean %>%
  group_by(Modification_Type, Value_Mofication) %>%
  summarise(
    mean_time = mean(Parsing_Time_ms, na.rm = TRUE),
    sd_time = sd(Parsing_Time_ms, na.rm = TRUE),
    n = n(),
    se_time = sd_time / sqrt(n),
    .groups = 'drop'
  ) %>%
  ggplot(aes(x = Value_Mofication, y = mean_time, color = Modification_Type)) +
  geom_line(linewidth = 1.2) +
  geom_point(size = 2) +
  geom_errorbar(aes(ymin = mean_time - se_time, 
                    ymax = mean_time + se_time), 
                width = 0.1, alpha = 0.7) +
  labs(
    title = "Evolution du Temps de Parsing par Type de Bruit",
    subtitle = "Erreur standard affichée",
    x = "Niveau de Modification",
    y = "Temps de Parsing (ms)",
    color = "Type de Modification"
  ) +
  theme_minimal() +
  theme(legend.position = "bottom")

# Graphique 4: Boxplots du temps de parsing par niveau de bruit
p4 <- data_clean %>%
  ggplot(aes(x = factor(Value_Mofication), y = Parsing_Time_ms, 
             fill = Modification_Type)) +
  geom_boxplot(alpha = 0.7, outlier.alpha = 0.5) +
  facet_wrap(~ Modification_Type, scales = "free") +
  labs(
    title = "Distribution du Temps de Parsing par Niveau et Type de Bruit",
    x = "Niveau de Modification",
    y = "Temps de Parsing (ms)",
    fill = "Type de Modification"
  ) +
  theme_minimal() +
  theme(
    legend.position = "none",
    axis.text.x = element_text(angle = 45, hjust = 1)
  )

# ===========================================
# 5. ANALYSE PAR TYPE DE PARSER (BONUS)
# ===========================================

# Graphique 5: Comparaison des performances par type de parser
p5 <- data_clean %>%
  group_by(Parser_Type, Modification_Type, Value_Mofication) %>%
  summarise(
    mean_precision = mean(Precision_Error_Avg_px, na.rm = TRUE),
    mean_time = mean(Parsing_Time_ms, na.rm = TRUE),
    .groups = 'drop'
  ) %>%
  ggplot(aes(x = Value_Mofication, y = mean_precision, 
             color = Parser_Type, linetype = Modification_Type)) +
  geom_line(linewidth = 1) +
  geom_point(size = 1.5) +
  labs(
    title = "Comparaison de la Précision par Type de Parser",
    x = "Niveau de Modification",
    y = "Erreur de Précision Moyenne (px)",
    color = "Type de Parser",
    linetype = "Type de Modification"
  ) +
  theme_minimal() +
  theme(legend.position = "bottom")

# Graphique 6: Heatmap de corrélation précision vs temps
p6 <- data_clean %>%
  ggplot(aes(x = Parsing_Time_ms, y = Precision_Error_Avg_px, 
             color = Modification_Type)) +
  geom_point(alpha = 0.6, size = 1.5) +
  geom_smooth(method = "lm", se = TRUE, alpha = 0.3) +
  facet_wrap(~ Modification_Type) +
  labs(
    title = "Relation entre Temps de Parsing et Précision",
    x = "Temps de Parsing (ms)",
    y = "Erreur de Précision Moyenne (px)",
    color = "Type de Modification"
  ) +
  theme_minimal() +
  theme(legend.position = "none")

# ===========================================
# 6. AFFICHAGE DES GRAPHIQUES
# ===========================================

# Affichage des graphiques principaux
print("=== PRECISION MOYENNE ===")
print(p1)
print(p2)

print("=== TEMPS DE PARSING ===")
print(p3)
print(p4)

print("=== ANALYSES COMPLEMENTAIRES ===")
print(p5)
print(p6)

# Graphique combiné avec grid.arrange
combined_plot <- grid.arrange(p1, p3, p2, p4, ncol = 2, nrow = 2,
                             top = "Analyse des Performances du Parser")

# ===========================================
# 7. ANALYSES STATISTIQUES COMPLEMENTAIRES
# ===========================================

# Test de corrélation entre précision et temps (avec gestion d'erreur)
if(nrow(data_clean) > 2) {
  cor_test <- tryCatch({
    cor.test(data_clean$Precision_Error_Avg_px, data_clean$Parsing_Time_ms)
  }, error = function(e) {
    cat("Erreur lors du test de corrélation:", e$message, "\n")
    NULL
  })
  
  if(!is.null(cor_test)) {
    cat("\nCorrélation Précision-Temps:\n")
    cat("Corrélation de Pearson:", round(cor_test$estimate, 3), "\n")
    cat("p-value:", format(cor_test$p.value, scientific = TRUE), "\n")
  }
} else {
  cat("\nPas assez de données pour le test de corrélation\n")
  cor_test <- NULL
}

# Analyse de variance pour tester l'effet du type de modification (avec gestion d'erreur)
if(nrow(data_clean) > 5 && length(unique(data_clean$Modification_Type)) > 1) {
  anova_precision <- tryCatch({
    aov(Precision_Error_Avg_px ~ Modification_Type * Value_Mofication, data = data_clean)
  }, error = function(e) {
    cat("Erreur ANOVA précision:", e$message, "\n")
    NULL
  })
  
  anova_time <- tryCatch({
    aov(Parsing_Time_ms ~ Modification_Type * Value_Mofication, data = data_clean)
  }, error = function(e) {
    cat("Erreur ANOVA temps:", e$message, "\n")
    NULL
  })
  
  if(!is.null(anova_precision)) {
    cat("\nANOVA - Effet sur la précision:\n")
    print(summary(anova_precision))
  }
  
  if(!is.null(anova_time)) {
    cat("\nANOVA - Effet sur le temps:\n")
    print(summary(anova_time))
  }
} else {
  cat("\nPas assez de données pour l'analyse ANOVA\n")
  anova_precision <- NULL
  anova_time <- NULL
}

# Résumé des tendances par type de modification (avec gestion d'erreur)
trends_summary <- data_clean %>%
  group_by(Modification_Type) %>%
  summarise(
    n_obs = n(),
    .groups = 'drop'
  ) %>%
  filter(n_obs >= 3) %>%  # Au moins 3 observations pour un modèle
  left_join(
    data_clean %>%
      group_by(Modification_Type) %>%
      filter(n() >= 3) %>%
      do({
        tryCatch({
          model_precision <- lm(Precision_Error_Avg_px ~ Value_Mofication, data = .)
          model_time <- lm(Parsing_Time_ms ~ Value_Mofication, data = .)
          data.frame(
            slope_precision = coef(model_precision)[2],
            slope_time = coef(model_time)[2],
            r2_precision = summary(model_precision)$r.squared,
            r2_time = summary(model_time)$r.squared
          )
        }, error = function(e) {
          data.frame(
            slope_precision = NA,
            slope_time = NA,
            r2_precision = NA,
            r2_time = NA
          )
        })
      }),
    by = "Modification_Type"
  )

cat("\nTendances (pentes) par type de modification:\n")
print(trends_summary)

# ===========================================
# 8. SAUVEGARDE DES RESULTATS
# ===========================================

# Création du dossier de sortie pour les graphiques
if (!dir.exists("limite")) {
  dir.create("limite")
  cat("Dossier 'limite' créé pour stocker les graphiques.\n")
}

# Sauvegarde des graphiques en haute résolution (avec gestion d'erreur)
tryCatch({
  ggsave("limite/precision_evolution.png", p1, width = 12, height = 8, dpi = 300)
  ggsave("limite/precision_boxplots.png", p2, width = 12, height = 8, dpi = 300)
  ggsave("limite/time_evolution.png", p3, width = 12, height = 8, dpi = 300)
  ggsave("limite/time_boxplots.png", p4, width = 12, height = 8, dpi = 300)
  ggsave("limite/parser_comparison.png", p5, width = 12, height = 8, dpi = 300)
  ggsave("limite/precision_time_correlation.png", p6, width = 12, height = 8, dpi = 300)
  cat("Graphiques sauvegardés avec succès dans le dossier 'limite'.\n")
}, error = function(e) {
  cat("Erreur lors de la sauvegarde des graphiques:", e$message, "\n")
})

# Sauvegarde des statistiques dans un fichier (avec gestion d'erreur)
tryCatch({
  sink("analyse_statistiques.txt")
  cat("=== ANALYSE DES PERFORMANCES DU PARSER ===\n\n")
  cat("Statistiques descriptives par type de modification:\n")
  print(stats_by_type)
  
  if(!is.null(cor_test)) {
    cat("\n\nCorrélation Précision-Temps:\n")
    print(cor_test)
  }
  
  if(!is.null(anova_precision)) {
    cat("\n\nANOVA - Effet sur la précision:\n")
    print(summary(anova_precision))
  }
  
  if(!is.null(anova_time)) {
    cat("\n\nANOVA - Effet sur le temps:\n")
    print(summary(anova_time))
  }
  
  cat("\n\nTendances par type de modification:\n")
  print(trends_summary)
  sink()
}, error = function(e) {
  cat("Erreur lors de la sauvegarde des statistiques:", e$message, "\n")
})

cat("\nAnalyse terminée ! Graphiques et statistiques sauvegardés.\n")