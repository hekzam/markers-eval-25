# Hekzam Markers - Optimisation des marqueurs pour la correction d'examens

## Description du projet

Hekzam est un ensemble de logiciels conçu pour permettre la création et la correction automatisée ou semi-automatisée des examens papier.  
Actuellement, Hekzam utilise des **QR codes** comme marqueurs de bord pour la calibration et l'identification des pages des copies d'examen. Cependant, ces marqueurs présentent plusieurs inconvénients :

- **Coût élevé en encre** : les QR codes sont denses et nécessitent une quantité importante d'encre.
- **Espace occupé important** : ils réduisent la place disponible pour le contenu pédagogique.
- **Consommation de ressources computationnelles** : leur génération et leur reconnaissance demandent un traitement relativement lourd.

### Objectif du projet

L'objectif de ce projet est de **développer et évaluer des alternatives aux QR codes** utilisées actuellement dans Hekzam.  
Les nouvelles solutions devront être plus sobres en encre et en espace tout en maintenant une **grande robustesse** dans leur détection.

## Dépendances nécessaires

Avant de compiler et d'exécuter le projet, installez les dépendances suivantes :

```sh
sudo apt-get install cmake ninja-build libopencv-dev nlohmann-json3-dev snapd
sudo snap install typst
```

- `cmake` : Outil de génération de build.
- `ninja-build` : Système de compilation rapide.
- `libopencv-dev` : Bibliothèque de traitement d’image pour l’alignement et la reconnaissance des marqueurs.
- `nlohmann-json3-dev` : Gestion du format JSON pour la structuration des données.
- `snapd` & `typst` : Génération des formulaires d'examen via Typst.
- `libzbar-dev` & `libzbar0` : si vous souhaitez utiliser zbar pour la détection des QR codes au lieu de ZXing.

## Compilation du projet

Une fois les dépendances installées, compilez le projet avec :

```sh
cmake -H. -Bbuild-cmake -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake -j
```

- `-H.` : Spécifie le répertoire source.
- `-Bbuild-cmake` : Définit le répertoire de build.
- `-GNinja` : Utilisation de Ninja comme générateur de build.
- `-DCMAKE_BUILD_TYPE=Release` : Compilation optimisée.
- `-DENABLE_ZBAR=ON` : si vous souhaitez utiliser zbar pour la détection des QR codes au lieu de ZXing.

## 📄 Génération de copie

Une fois la compilation terminée, utilisez la commande suivante pour générer les copies :

```sh
./create-copie.sh [options]
```

Ce script permet de produire une copie vers le dossier de sortie **copies/**.

### Options disponibles

```
  --encoded-size N      : Taille des marqueurs encodés (par défaut: 15)
  --unencoded-size N     : Taille des marqueurs non encodés (par défaut: 3)
  --header-size N       : Taille du marqueur d'entête (par défaut: 7)
  --stroke-width N      : Largeur du trait des marqueurs (par défaut: 2)
  --margin N            : Marge autour des marqueurs (par défaut: 3)
  --grey-level N        : Niveau de gris (0: noir, 255: blanc) (par défaut: 0)
  --dpi N               : Résolution en points par pouce (par défaut: 300)
  --generating-content BOOL : Générer le contenu dans le document (1/true ou 0/false) (par défaut: 1)
  --filename NAME       : Nom du fichier de sortie (par défaut: copy)
  
  Options de configuration personnalisée des marqueurs:
  --tl TYPE             : Type de marqueur pour le coin supérieur gauche
  --tr TYPE             : Type de marqueur pour le coin supérieur droit
  --bl TYPE             : Type de marqueur pour le coin inférieur gauche
  --br TYPE             : Type de marqueur pour le coin inférieur droit
  --header TYPE         : Type de marqueur pour l'en-tête

  Format des types de marqueurs: type[:encoded][:outlined]
  - type:outlined     : Marqueur non rempli (Ne fonctionne que pour les formes géométriques simples)
  - type:encoded      : Marqueur avec données encodées
  - type:unencoded    : Marqueur sans données encodées
```

Exemple avec une configuration complète personnalisée:
```sh
./create-copie.sh --tl circle:outlined --tr circle:outlined --bl none --br qrcode:encoded --header qrcode:encoded --encoded-size 20 --unencoded-size 12 --grey-level 80 --header-size 18 --dpi 600 --filename exam_high_res
```

### Marqueurs disponibles

| Encodable       | Non encodable     | Rectangulaire    |
|-----------------|-------------------|------------------|
| qrcode          | circle            | pdf417           |
| micro-qr        | square            | rmqr             |
| datamatrix      | triangle          | code128          |
| aztec           | cross             |                  |
| pdf417          | aruco             |                  |
| rmqr            | qr-eye            |                  |
| code128         | custom            |                  |

## 📊 Exécution du benchmark

Vous pouvez exécuter l'outil de benchmark pour évaluer les performances des différentes configurations de marqueurs :

```sh
./run_benchmark.sh [options]
```

### Benchmarks disponibles

Le système propose plusieurs types de benchmarks, sélectionnables avec l'option `--benchmark` :

- `parsing-time` : Évalue les performances d'analyse et de détection des marqueurs
- `generation-time` : Évalue les performances de génération des copies (par défaut)
- `ink-estimation` : Estime la consommation d'encre pour différentes configurations de marqueurs

### Options en ligne de commande

Vous pouvez passer les paramètres directement en ligne de commande :

```sh
./run_benchmark.sh --benchmark ink-estimation --input-dir=./copies --dpi=600
```

Options communes :
- `--benchmark=<type>` : Type de benchmark à exécuter (parsing-time, generation-time, ink-estimation)
- `--output-dir=<path>` : Répertoire de sortie pour les résultats
- `--input-dir=<path>` : Répertoire contenant les images d'entrée
- `--encoded-marker_size=<N>` : Taille des marqueurs encodés en mm
- `--unencoded-marker_size=<N>` : Taille des marqueurs non encodés en mm
- `--header-marker_size=<N>` : Taille du marqueur d'en-tête en mm
- `--grey-level=<0-255>` : Niveau de gris pour les marqueurs
- `--dpi=<N>` : Résolution en points par pouce

Options spécifiques pour les benchmarks de performance :
- `--nb-copies=<N>` : Nombre de copies à générer pour le test
- `--warmup-iterations=<N>` : Nombre d'itérations d'échauffement avant la mesure
- `--atomic-boxes-file=<path>` : Fichier JSON contenant les définitions des zones

### Résultats des benchmarks

#### Benchmark de temps de traitement

Le benchmark `parsing-time` produit des résultats sur le temps de traitement et le taux de succès de la détection des marqueurs. Il génère les fichiers suivants dans le répertoire de sortie :
- **Images calibrées** : Versions redressées des copies avec les zones détectées
- **CSV de résultats** : Fichier `benchmark_results.csv` avec les temps d'exécution et taux de succès
- **Images de débogage** (mode DEBUG uniquement) : Visualisation du processus de détection

#### Benchmark d'estimation d'encre

Le benchmark `ink-estimation` analyse la consommation d'encre et affiche :
- Dimensions de l'image et résolution
- Surface totale couverte en cm²
- Couverture d'encre moyenne en pourcentage
- Volume d'encre estimé en millilitres
- Facteur de calibration utilisé

#### Benchmark de temps de génération de copies
Le benchmark `generation-time` évalue le temps de génération des copies. Il génère un fichier CSV avec les résultats de chaque itération.

L'option `--warmup-iterations` est particulièrement utile pour obtenir des mesures plus précises. Les itérations d'échauffement exécutent le même code que les itérations de mesure, mais leurs résultats ne sont pas comptabilisés dans les statistiques finales. Cela permet d'éviter que les coûts de démarrage (chargement initial des bibliothèques, initialisation des caches, etc.) n'affectent les mesures de performance.

### Résultats du benchmark

Après l'exécution, le benchmark produit plusieurs types de sorties :

- **Images calibrées** : Versions redressées des copies scannées avec les zones détectées surlignées
- **CSV de résultats** : Fichier `benchmark_results.csv` contenant les temps d'exécution et taux de succès pour chaque image
- **Images de débogage** (si compilé en mode DEBUG) : Visualisation du processus de détection des marqueurs

Le fichier CSV contient trois colonnes:
- **File**: Nom du fichier traité
- **Time(ms)**: Temps d'exécution en millisecondes
- **Success**: Indique si la détection des marqueurs a réussi (1) ou échoué (0)

Ces données vous permettent d'analyser:
- Le taux de succès global de la détection pour chaque configuration de marqueurs
- Le temps moyen de traitement
- L'impact des différents paramètres (taille, niveau de gris, etc.) sur les performances

Les images calibrées montrent les zones détectées avec les codes couleur suivants:
- **Rose**: Zones utilisateur (zones de réponse)
- **Bleu**: Marqueurs de coin
- **Vert**: Centre des marqueurs de coin

## 📂 Structure du projet

```
.
├── include/            # Fichiers d'en-tête (*.h, *.hpp)
│   ├── benchmark.hpp   # En-têtes pour le benchmarking
│   └── common.h        # Définitions de structures communes
├── src/                # Code source C++ principal
│   ├── bench/          # Code source des benchmarks
│   ├── benchmark.cpp   # Outil de benchmarking
│   ├── expl_pars.cpp   # Parseur principal
│   ├── typst_interface.cpp # Interface avec Typst
│   ├── utils/          # Utilitaires partagés
│   ├── parser/         # Implémentation des parseurs de marqueurs
│   └── external-tools/ # Outils externes (création de copies)
├── typst/              # Sources de templates Typst
│   ├── components/     # Composants réutilisables (marqueurs, conteneurs)
│   ├── common/         # Variables et utilitaires communs
│   ├── content/        # Contenu des formulaires
│   ├── src/            # Scripts de génération
│   ├── style/          # Configuration de style
│   └── template.typ    # Template principal
├── stats-analysis/     # Scripts et outils d'analyse statistique
├── copies/             # Dossier de sortie pour les copies générées
├── output/             # Dossier de sortie pour les résultats d'analyse
├── build-cmake/        # Répertoire de build (généré)
├── CMakeLists.txt      # Configuration du projet CMake
├── create-copie.sh     # Script de génération de copies
├── run_benchmark.sh    # Script d'exécution du benchmark
├── README.md           # Ce fichier
└── LICENSE             # Fichier de licence
```

## 📖 Références techniques

- **OpenCV** : [https://opencv.org/](https://opencv.org/)
- **Typst** : [https://typst.app/](https://typst.app/)
- **ZXing** : [https://github.com/zxing/zxing](https://github.com/zxing/zxing)

## ⚖️ License

- Code: Apache-2.0
- Everything else, in particular documentation and measurements: CC-BY-SA-4.0
