# 📊 Outils d'analyse statistique des benchmarks

Ce dossier contient les outils et scripts nécessaires pour analyser les données de benchmark des différents marqueurs et parseurs.

## Description des outils

Les outils d'analyse statistique permettent d'évaluer les performances et la fiabilité des différents parseurs et configurations de marqueurs. Ils génèrent des visualisations et des métriques qui aident à comparer objectivement les solutions.

## Exécution de l'analyse

### Prérequis

Avant d'exécuter les analyses, assurez-vous d'avoir installé :

- Python 3.6+
- R avec les packages suivants :
  - ggplot2
  - dplyr
  - tidyr
  - fs

Pour installer les packages R nécessaires, exécutez dans R :

```r
install.packages(c("ggplot2", "dplyr", "tidyr", "fs"))
```

### Lancement rapide

La façon la plus simple d'exécuter l'analyse est d'utiliser le script launcher.py :

```sh
python launcher.py --csv chemin/vers/votre_fichier.csv
```

Si aucun fichier CSV n'est spécifié, le script utilisera par défaut `csv_by_parser.csv` dans le répertoire courant.

### Structure des fichiers CSV

L'analyse attend un fichier CSV avec au moins les colonnes suivantes :

- `File` : Le nom du fichier image analysé
- `Parser` : Le type de parseur utilisé (ex: qrcode_parser, circle_parser, etc.)
- `Time_ms` : Le temps d'exécution en millisecondes
- `Noise_Level` (optionnel) : Le niveau de bruit appliqué à l'image

## 📈 Types d'analyses disponibles

### 1. Analyse par parseur

Pour chaque parseur, les scripts génèrent :

- Des graphiques de temps d'exécution pour chaque copie
- Des comparaisons avec la moyenne et l'écart-type
- Des histogrammes de distribution des temps
- Des boxplots par groupe de copies
- Des analyses d'impact du bruit (si les données sont disponibles)

### 2. Analyse comparative

Les scripts génèrent également des comparaisons entre les différents parseurs :

- Graphiques de temps moyens par parseur
- Boxplots pour comparer les distributions
- Analyses de l'impact du bruit sur les différents parseurs
- Tableaux statistiques comparatifs

## 🔍 Description des scripts

### intra_parser_analysis.r

Script R principal qui effectue l'analyse détaillée des performances par parseur et la comparaison entre parseurs.

Fonctionnalités principales :
- Création de visualisations pour chaque parseur
- Calcul de statistiques descriptives (moyenne, médiane, écart-type)
- Analyse de l'impact du bruit sur les performances
- Comparaison des performances entre les différents parseurs

### launcher.py

Script Python qui sert d'interface pour lancer facilement l'analyse R avec différents fichiers CSV.

Options :
- `--csv` : Spécifier un fichier CSV alternatif (par défaut: `csv_by_parser.csv`)

## 📁 Structure des résultats

Les résultats sont organisés dans un dossier `analysis_results` avec la structure suivante :

```
analysis_results/
├── aruco_parser/
│   ├── aruco_parser_temps_execution_copies.png
│   ├── aruco_parser_comparaison_moyenne.png
│   ├── aruco_parser_distribution_temps.png
│   ├── aruco_parser_comparison_groupes.png
│   ├── aruco_parser_impact_bruit_temps.png (si applicable)
│   └── aruco_parser_statistiques.txt
├── circle_parser/
│   └── ...
├── ...
└── comparaison_parseurs/
    ├── comparaison_temps_moyens.png
    ├── distribution_temps_parseurs.png
    ├── effet_bruit_parseurs.png (si applicable)
    └── statistiques_parseurs.csv
```

## 📊 Exemples de visualisations générées

### Par parseur
- **Temps d'exécution par copie** : Graphique linéaire montrant le temps d'exécution pour chaque copie.
- **Comparaison avec la moyenne** : Graphique comparant les temps individuels avec la moyenne et affichant l'écart-type.
- **Distribution des temps** : Histogramme des temps d'exécution.
- **Analyse par groupe** : Boxplot des temps d'exécution par groupe de copies.
- **Impact du bruit** : Graphique montrant l'effet du niveau de bruit sur les temps d'exécution.

### Comparaison entre parseurs
- **Temps moyens** : Graphique à barres comparant les temps moyens de chaque parseur.
- **Distribution** : Boxplot comparant les distributions des temps d'exécution entre parseurs.
- **Effet du bruit** : Graphique linéaire montrant l'évolution des temps d'exécution selon le niveau de bruit pour chaque parseur.

## 🛠️ Extension des analyses

Pour ajouter un nouveau type d'analyse :

1. Créez un nouveau script R dans ce répertoire
2. Mettez à jour le script launcher.py pour inclure votre nouveau script
3. Documentez les nouvelles fonctionnalités dans ce README

## 📑 Interprétation des résultats

### Métriques clés

- **Temps d'exécution moyen** : Indique la performance générale du parseur.
- **Écart-type** : Mesure la consistance des performances.
- **Taux de succès** : Pourcentage de copies correctement analysées.
- **Résistance au bruit** : Évaluée par l'évolution des performances en fonction du niveau de bruit.

### Conseils d'interprétation

- Un parseur idéal combine un temps d'exécution faible, un écart-type bas et une bonne résistance au bruit.
- Évaluez le compromis entre la vitesse d'exécution et la fiabilité selon votre cas d'utilisation.
- Tenez compte du type de marqueur utilisé dans l'interprétation des résultats (certains parseurs sont optimisés pour des types spécifiques de marqueurs).

## ⚠️ Limitations connues

- L'analyse ne tient pas compte de la qualité de la détection (précision des coins détectés).
- Les performances peuvent varier en fonction du matériel utilisé pour les benchmarks.
- Les résultats sont spécifiques aux configurations testées et peuvent ne pas être généralisables.