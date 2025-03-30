# 📊 Analyse des Temps d'Exécution des Copies d'Images

Ce document présente une analyse détaillée des performances temporelles lors de la copie d'images, générée par le script `Lecture_csv.r`. L'analyse vise à identifier les variations de performance et les tendances dans les temps d'exécution.

## 📋 Sommaire
- [Vue d'ensemble](#vue-densemble)
- [Visualisations](#visualisations)
- [Données source](#données-source)
- [Installation et exécution](#installation-et-exécution)
- [Notes techniques](#notes-techniques)

## 🔍 Vue d'ensemble

Cette analyse examine les temps d'exécution de n copies d'images pour identifier les variations de performance. Le script R génère 4 visualisations complémentaires qui offrent différentes perspectives sur les données de performance, permettant d'identifier les tendances, les valeurs aberrantes et les performances relatives.

## 📈 Visualisations

### 1. Évolution temporelle
![Temps d'exécution pour chaque copie](temps_execution_copies.png)

**Objectif**: Observer l'évolution chronologique des performances  
**Analyse**:
- Montre les fluctuations de temps d'exécution entre chaque copie successive
- Permet d'identifier visuellement les pics et les creux de performance
- La première copie (copy01.png) présente un temps d'exécution significativement plus élevé que les autres

### 2. Comparaison avec la moyenne
![Comparaison des temps par rapport à la moyenne](comparaison_moyenne.png)

**Objectif**: Situer chaque copie par rapport à la performance moyenne  
**Analyse**:
- La ligne pointillée rouge représente le temps d'exécution moyen
- La zone bleutée délimite l'intervalle défini par l'écart-type (±σ)
- Les points verts indiquent des performances meilleures que la moyenne
- Les points rouges indiquent des performances moins bonnes que la moyenne
- Permet d'identifier rapidement les valeurs aberrantes

### 3. Distribution statistique
![Distribution des temps d'exécution](distribution_temps.png)

**Objectif**: Comprendre la répartition statistique des temps d'exécution  
**Analyse**:
- L'histogramme montre la fréquence d'apparition des différentes plages de temps
- La ligne rouge indique la moyenne des temps d'exécution
- Permet d'évaluer si la distribution est normale ou présente une asymétrie
- Aide à déterminer si certaines plages de temps sont plus fréquentes que d'autres

### 4. Analyse par groupes
![Comparaison des temps d'exécution par groupe de copies](comparison_groupes.png)

**Objectif**: Comparer les performances entre différents groupes de copies  
**Analyse**:
- Divise les 15 copies en trois groupes de 5 (1-5, 6-10, 11-15)
- Les boîtes à moustaches montrent la distribution intra-groupe (médiane, quartiles, valeurs extrêmes)
- Les points représentent les valeurs individuelles
- Permet d'identifier d'éventuelles tendances ou améliorations par groupe
- Aide à déterminer si les performances varient selon la séquence d'exécution

## 📄 Données source

Le fichier `benchmark_performance.csv` utilisé pour cette analyse contient les données suivantes:

| File       | Time(ms) | Success |
|------------|----------|---------|
| copy01.png | 67.103   | 1       |
| copy02.png | 36.476   | 1       |
| copy03.png | 33.642   | 1       |
| ...        | ...      | ...     |
| copyn.png  | ...      | ...     |

**Structure des données**:
- **File**: Nom du fichier image copié
- **Time(ms)**: Temps d'exécution en millisecondes
- **Success**: Indicateur de réussite (1 = succès)

## 🚀 Installation et exécution

### Prérequis
- R (version 3.6.0 ou supérieure)
- Packages R: `ggplot2`, `dplyr`, `tidyr`, `corrplot`

### Installation des packages
```r
install.packages(c("ggplot2", "dplyr", "tidyr", "corrplot"))
```

### Exécution de l'analyse ( nb pour mes camarades : peut etre a changer )
1. Placez le fichier `benchmark_performance.csv` dans le même dossier que le script ou dans le chemin spécifié
2. Ouvrez R ou RStudio
3. Définissez le répertoire de travail correct:
   ```r
   setwd("chemin/vers/votre/dossier")
   ```
4. Exécutez le script:
   ```r
   source("Lecture_csv.r")
   ```
5. Les graphiques générés seront sauvegardés dans votre répertoire de travail


## 📝 Notes techniques

### Méthodes statistiques utilisées
- **Moyenne et écart-type**: Pour établir les références de performance
- **Boîtes à moustaches**: Pour visualiser la distribution par groupe

*Dernière mise à jour: Mars 2025*