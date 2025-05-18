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
sudo apt-get install cmake ninja-build libopencv-dev nlohmann-json3-dev snapd doxygen graphviz
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
cmake -H. -Bbuild-cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DAUTO_DOWNLOAD_ZXING=ON
cmake --build build-cmake -j
```

- `-H.` : Spécifie le répertoire source.
- `-Bbuild-cmake` : Définit le répertoire de build.
- `-GNinja` : Utilisation de Ninja comme générateur de build.
- `-DCMAKE_BUILD_TYPE=Release` : Compilation optimisée.
- `-DENABLE_ZBAR=ON` : si vous souhaitez utiliser zbar pour la détection des QR codes au lieu de ZXing.

## Utilisation de Nix

Ce projet prend également en charge [Nix](https://nixos.org/) pour gérer les dépendances et créer des environnements de développement reproductibles, ce qui peut simplifier l'installation et la configuration.

### Utilisation du build system Meson avec Nix

Ce projet utilise systématiquement le système de build [Meson](https://mesonbuild.com/) lorsqu'il est compilé dans un environnement Nix. Contrairement à l'utilisation de CMake décrite ci-dessus, nix-shell repose exclusivement sur Meson pour la compilation du projet.

### Environnements de développement Nix

Le projet propose plusieurs environnements de développement Nix, tous utilisant Meson comme système de build :

#### Utilisation avec les flakes

```sh
# Entrer dans l'environnement Nix
nix develop

# Configurer le projet avec Meson (utilisé automatiquement par nix-shell)
meson setup build

# Compiler le projet
meson compile -C build

# Les exécutables seront disponibles dans le dossier `build`
```

#### Utilisation sans flakes

Pour ceux qui préfèrent ne pas utiliser les flakes :

```sh
# Environnement de développement principal (utilise Meson pour la compilation)
nix-shell --pure

# Environnement pour l'analyse statistique
cd stats-analysis && nix-shell --pure
```

## Génération de la documentation

Ce projet utilise Doxygen pour générer sa documentation technique. La documentation inclut des détails sur les classes, les fonctions et les structures de données utilisées dans le projet.

Pour générer la documentation, exécutez la commande suivante depuis la racine du projet :

```sh
# Si vous utilisez CMake
cmake --build build-cmake --target docs

# Si vous utilisez Meson
ninja -C build docs
```

Alternativement, vous pouvez utiliser directement Doxygen avec le fichier de configuration :

```sh
doxygen Doxyfile
```

### Consultation de la documentation

Une fois générée, la documentation HTML sera disponible dans le répertoire `html/`.

## Génération de copie

Une fois la compilation terminée, vous pouvez générer des copies d'examen avec différents types de marqueurs.

Utilisez la commande suivante pour générer des copies avec des options personnalisées :

```sh
./build-cmake/create-copie [options]
```

Les copies générées sont sauvegardées dans le dossier **copies/**.

### Types de marqueurs disponibles

| Encodable       | Non encodable     | Rectangulaire    |
|-----------------|-------------------|------------------|
| qrcode          | circle            | pdf417           |
| microqr         | square            | rmqr             |
| datamatrix      | cross             | code128          |
| aztec           | aruco             |                  |
| pdf417          | qreye             |                  |
| rmqr            | custom            |                  |
| code128         |                   |                  |

### Options de configuration

- `--encoded-size <N>`          : Taille des marqueurs encodés (par défaut: 15)
- `--unencoded-size <N>`        : Taille des marqueurs non encodés (par défaut: 3)
- `--header-size <N>`           : Taille du marqueur d'entête (par défaut: 7)
- `--stroke-width <N>`          : Largeur du trait des marqueurs (par défaut: 2)
- `--marker-margin <N>`                : Marge autour des marqueurs (par défaut: 3)
- `--content-margin-x <N>`      : Marge horizontale pour le contenu en mm (par défaut: 10)
- `--content-margin-y <N>`      : Marge verticale pour le contenu en mm (par défaut: 10)
- `--grey-level <N>`            : Niveau de gris (0: noir, 255: blanc) (par défaut: 0)
- `--dpi <N>`                   : Résolution en points par pouce (par défaut: 300)
- `--generating-content <BOOL>` : Générer le contenu dans le document (1/true ou 0/false) (par défaut: 1)
- `--seed <N>`                  : Graine pour la génération aléatoire du contenu (par défaut: 42)
- `--filename <name>`           : Nom du fichier de sortie (par défaut: copy)
- `--tl <type>`                 : Type de marqueur pour le coin supérieur gauche (par défaut: qrcode:encoded)
- `--tr <type>`                 : Type de marqueur pour le coin supérieur droit (par défaut: qrcode:encoded)
- `--bl <type>`                 : Type de marqueur pour le coin inférieur gauche (par défaut: qrcode:encoded)
- `--br <type>`                 : Type de marqueur pour le coin inférieur droit (par défaut: qrcode:encoded)
- `--header <type>`             : Type de marqueur pour l'en-tête (par défaut: none)
- `--verbose`                   : Affiche tous les messages de sortie des commandes de typst (par défaut: non affiché)

#### Format des types de marqueurs

```
  Format: type[:encoded][:outlined]
  - type:outlined     : Marqueur non rempli (uniquement pour formes géométriques simples)
  - type:encoded      : Marqueur avec données encodées
  - none              : Aucun marqueur à cette position
```

> **Note sur l'encodage :** Par défaut, les marqueurs encodables contiennent uniquement l'information de leur position (coin supérieur gauche, supérieur droit, etc.). Avec l'option `:encoded`, le marqueur encodera également le numéro de la page, de la copie ainsi que le nom de l'examen.

### Exemples

#### Exemple simple avec des QR codes
```sh
./build-cmake/create-copie --tl qrcode --tr qrcode --bl qrcode --br qrcode --header qrcode
```

#### Configuration avancée avec différents marqueurs
```sh
./build-cmake/create-copie --tl circle:outlined --tr circle:outlined --bl none --br qrcode:encoded --header qrcode:encoded --encoded-size 20 --unencoded-size 12 --grey-level 80 --header-size 18 --content-margin-x 15 --content-margin-y 25 --seed 123 --dpi 600 --filename exam_high_res
```

### Flux de travail de génération

1. Le processus commence par l'exécution du programme `create-copie` qui analyse les arguments de la ligne de commande via `gen_copies.cpp`.
2. Les arguments sont convertis en paramètres de style (`CopyStyleParams`) et en configuration de marqueurs (`CopyMarkerConfig`).
3. La fonction `create_copy` dans `create_copy.cpp` est alors appelée avec ces paramètres.
4. Cette fonction:
   - Crée les répertoires nécessaires (`./copies` et `./copies/metadata`)
   - Construit une chaîne de paramètres à transmettre à Typst
   - Exécute la commande `typst compile` pour générer le fichier PDF/PNG
   - Exécute la commande `typst query` pour extraire les métadonnées (positions des zones/marqueurs)
5. Les métadonnées sont sauvegardées dans `./copies/metadata/` au format JSON pour une utilisation ultérieure.

## Outil d'analyse d'images (expl_pars)

Le programme `expl_pars` (Exploratory Parser) est un outil d'analyse d'images qui permet de détecter les marqueurs, redresser les images, et extraire des zones spécifiques pour leur traitement ultérieur.

### Fonctionnalités principales

- **Détection de marqueurs** : Identifie les marqueurs placés sur les coins des documents
- **Redressement d'images** : Applique une transformation affine pour corriger les déformations
- **Extraction de zones** : Extrait des zones précises définies dans un fichier JSON
- **Calibration automatique** : S'adapte à différents types de marqueurs grâce aux parseurs spécialisés

### Utilisation

```sh
./build-cmake/expl_pars <output_dir> <atomic_boxes.json> <image1> [image2] [image3] ...
```

- `output_dir` : Répertoire où seront enregistrés les résultats
- `atomic_boxes.json` : Fichier de définition des zones à extraire
- `image1, image2...` : Images à analyser (copies d'examens scannées)

### Résultats générés

Pour chaque image traitée, le programme produit :

1. **Image calibrée** : Une version redressée de l'image originale avec les zones d'intérêt surlignées, enregistrée dans le format `cal-<nom_original>.png`

2. **Sous-images extraites** : Des images individuelles pour chaque zone d'intérêt, enregistrées dans le sous-répertoire `subimg/` au format `raw-<id_copie>-<id_zone>.png`

### Format du fichier JSON de définition des zones

Le fichier JSON doit décrire les "boîtes atomiques" (zones d'intérêt) avec leurs coordonnées en millimètres sur la page :

```json
[
  {
    "id": "marker-tl",
    "x": 10,
    "y": 10,
    "width": 15,
    "height": 15,
    "page": 1,
    "type": "marker",
    "position": "tl"
  },
  {
    "id": "question1",
    "x": 50,
    "y": 70,
    "width": 100,
    "height": 30,
    "page": 1,
    "type": "question"
  }
  // ...autres zones
]
```

Les zones de type "marker" avec des positions "tl", "tr", "bl", "br" définissent les marqueurs de coin utilisés pour la calibration.

### Exemples d'utilisation

#### Analyse d'une copie individuelle

```sh
./build-cmake/expl_pars output/ description.json copies/copie_modifiee.png
```

#### Traitement par lot de toutes les copies d'un examen

```sh
./build-cmake/expl_pars output/ description.json copies/*.png
```

#### Utilisation avec des images modifiées

```sh
./build-cmake/modifier copies/copie1.png -r=5 -cb=10,5
```

## Exécution du benchmark

Le projet inclut plusieurs outils de benchmark pour évaluer différents aspects des marqueurs, comme leur consommation d'encre, leur facilité de détection, et leurs performances globales.

### Exécution des benchmarks

Vous disposez de deux méthodes pour exécuter les benchmarks :

#### 1. Mode ligne de commande (recommandé)

Spécifiez directement tous les paramètres dans votre commande :

```sh
./build-cmake/bench --benchmark [nom-du-benchmark] [autres-options]
```

Exemple :
```sh
./build-cmake/bench --benchmark gen-parse --nb-copies 5 --marker-config "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)" --parser-type QRCODE
```

#### 2. Mode interactif

Exécutez simplement la commande en spécifiant au minimum le type de benchmark :

```sh
./build-cmake/bench --benchmark [nom-du-benchmark]
```

Le programme vous guidera ensuite pour saisir les autres paramètres via une interface interactive dans le terminal.

> **Note** : Si vous ne spécifiez pas de type avec l'option `--benchmark`, le benchmark par défaut sera `config-analysis`.

#### 3. Mode batch

Vous pouvez également exécuter une série de benchmarks depuis un fichier texte contenant les commandes :

```sh
./build-cmake/bench --batch-file [chemin-vers-fichier]
```

Chaque ligne du fichier doit contenir un type de benchmark et ses options. Les lignes vides ou commençant par `#` sont ignorées.

Exemple de fichier batch.txt :
```
gen-parse --nb-copies 3 --marker-config "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)"
config-analysis --marker-config "(datamatrix:encoded,datamatrix:encoded,datamatrix:encoded,datamatrix:encoded,none)"
# Cette ligne est un commentaire
gen-parse --nb-copies 2 --marker-config "(circle:outlined,circle:outlined,circle:outlined,circle:outlined,none)"
```

#### Types de benchmark disponibles

Voici les différents types de benchmarks que vous pouvez exécuter :

1. **gen-parse** : Évalue le temps de génération des copies, le temps de traitement et le taux de succès de la détection des marqueurs. Il mesure également la précision de la rectification des copies après détection.
   ```sh
   ./build-cmake/bench --benchmark gen-parse
   ```

   **Options spécifiques** :
   - `--parser-type <type>` : Type de parseur à utiliser (QRCODE, ARUCO, CIRCLE, etc.)
   - `--nb-copies <N>` : Nombre de copies à générer et analyser
   - `--seed <N>` : Graine pour la génération aléatoire (0 pour une graine basée sur le temps)
   - `--warmup-iterations <N>` : Nombre d'itérations d'échauffement avant les mesures

2. **config-analysis** : Analyse la consommation d'encre et la surface occupée par les marqueurs.
   ```sh
   ./build-cmake/bench --benchmark config-analysis
   ```

   **Options spécifiques** :
   - `--calibration-factor <N>` : Facteur de calibration pour le calcul de consommation d'encre (ml/cm²)

### Types de parseurs disponibles

Le système prend en charge plusieurs types de parseurs pour la détection et le traitement des marqueurs. Lors de l'utilisation de l'option `--parser-type` dans les benchmarks, vous pouvez spécifier l'un des parseurs suivants :

1. **QRCODE** (par défaut) : Parseur standard pour les codes QR. Détecte les marqueurs QR code encodés avec l'identifiant de position (tl, tr, bl, br) et extrait les métadonnées.

2. **EMPTY** : Parseur pour les codes QR sans identification de position. Utilise la disposition pour déterminer quelle position est occupée par quel marqueur.

3. **CIRCLE** : Détecte les marqueurs circulaires et les utilise avec un QR code d'identification pour déterminer l'orientation.

4. **ARUCO** : Détecte les marqueurs ArUco (codes carrés spécifiques pour la réalité augmentée) et les utilise pour l'alignement. Nécessite un QR code pour les métadonnées.

5. **CENTER_MARKER_PARSER** : Détecte les marqueurs à partir du centre de la page, utile lorsque les marqueurs ne sont pas positionnés dans les coins.

6. **SHAPE** : Détecte les marqueurs basés sur des formes géométriques simples. Utilise un processus de détection des contours.

7. **DEFAULT_PARSER** : Implémentation par défaut (ne fait rien). Utile principalement à des fins de test ou comme point de départ pour de nouveaux parseurs.

Exemple d'utilisation avec un parseur spécifique :
```sh
./build-cmake/bench --benchmark gen-parse --parser-type CIRCLE --marker-config "(circle:outlined,circle:outlined,circle:outlined,circle:outlined,none)"
```

> **Note** : Tous les parseurs ne sont pas compatibles avec tous les types de marqueurs. Par exemple, le parseur CIRCLE ne fonctionnera correctement qu'avec des marqueurs de type cercle, et le parseur ARUCO avec des marqueurs ArUco.

### Options communes à tous les benchmarks

- `--marker-config <config>` : Configuration des marqueurs (par défaut: `(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)`)
  > Format: (tl,tr,bl,br,header) où tl=top-left, tr=top-right, bl=bottom-left, br=bottom-right, header=en-tête.
  > Utilisez "none" pour les positions qui ne sont pas occupées par des marqueurs.
- `--encoded-marker-size <N>` : Taille des marqueurs encodés en mm (par défaut: 13)
- `--unencoded-marker-size <N>` : Taille des marqueurs non encodés en mm (par défaut: 10)
- `--header-marker-size <N>` : Taille du marqueur d'en-tête en mm (par défaut: 7)
- `--grey-level <0-255>` : Niveau de gris pour les marqueurs (par défaut: 0)
- `--dpi <N>` : Résolution en points par pouce (par défaut: 300)
- `--csv-mode <mode>` : Mode de gestion des fichiers CSV existants :
  - `overwrite` : Supprime les fichiers CSV existants et crée un nouveau fichier (par défaut)
  - `append` : Conserve le fichier CSV existant et ajoute les nouveaux résultats à la fin
- `--csv-filename <nom>` : Nom du fichier CSV pour les résultats (par défaut: "benchmark_results.csv")

### Format des fichiers de résultats

Les résultats des benchmarks sont sauvegardés dans le dossier `output/csv/` au format CSV. Les fichiers incluent :

- Pour **gen-parse** : Noms des fichiers, temps de génération, temps de parsing, succès du parsing, type de parseur, configuration des marqueurs, erreurs de précision, etc.
- Pour **config-analysis** : Configuration des marqueurs, estimation de consommation d'encre, zone totale occupée par les marqueurs, etc.

### Exemples d'utilisation

#### Exemple 1 : Benchmark simple avec le parseur QR code
```sh
./build-cmake/bench --benchmark gen-parse --nb-copies 3 --parser-type QRCODE
```

#### Exemple 2 : Analyse de consommation d'encre pour un marqueur spécifique
```sh
./build-cmake/bench --benchmark config-analysis --marker-config "(circle:outlined,circle:outlined,circle:outlined,circle:outlined,none)" --grey-level 50
```

#### Exemple 3 : Benchmark complet avec options avancées
```sh
./build-cmake/bench --benchmark gen-parse --nb-copies 20 --parser-type SHAPE --marker-config "(square,square,square,qrcode:encoded,none)" --encoded-marker-size 20 --unencoded-marker-size 8 --warmup-iterations 2 --seed 42
```

## Module de modification d'images

Le projet inclut un module de simulation de défauts d'image qui permet de tester la robustesse des marqueurs dans des conditions réelles de numérisation. Ce module est particulièrement important pour évaluer la fiabilité des différents types de marqueurs face aux dégradations typiques qui surviennent lors de l'impression et de la numérisation des copies d'examen.

### Fonctionnalités de simulation

Le module offre plusieurs transformations et dégradations qui peuvent être appliquées individuellement ou combinées :

- **Bruit "sel et poivre"** : Ajoute des pixels noirs et blancs aléatoires
- **Bruit gaussien** : Simule le bruit de fond naturel avec une dispersion contrôlée
- **Contraste et luminosité** : Modifie les niveaux pour simuler les variations d'exposition
- **Taches d'encre** : Ajoute des marques similaires à celles que pourrait faire un étudiant
- **Rotations et translations** : Simule le désalignement lors de la numérisation
- **Effets d'impression** : Reproduit les artefacts typiques des imprimantes (tramage, lignes manquantes)
- **Compression JPEG** : Simule les artefacts de compression numérique

### Utilisation en ligne de commande

L'outil peut être utilisé directement en ligne de commande pour traiter des images individuelles :

```sh
./build-cmake/modifier <chemin_image> [options]
```

#### Modes d'utilisation

1. **Mode aléatoire simple** : Applique une combinaison aléatoire de toutes les transformations
   ```sh
   ./build-cmake/modifier input.jpg
   ```

2. **Mode aléatoire avec seed** : Applique des transformations aléatoires avec une graine spécifique
   ```sh
   ./build-cmake/modifier input.jpg -seed=123
   ```

3. **Mode coefficient de distorsion** : Applique une distorsion proportionnelle au coefficient
   ```sh
   ./build-cmake/modifier input.jpg -coef=0.5
   ```

4. **Mode transformations spécifiques** : Applique uniquement les transformations spécifiées
   ```sh
   ./build-cmake/modifier input.jpg -sp=2,3 -cb=50,10 -r=5
   ```

#### Options disponibles

- `-sp=salt,pepper` : Bruit sel et poivre (pourcentage de pixels blancs, pourcentage de pixels noirs)
- `-g=offset,dispersion` : Bruit gaussien (offset, dispersion)
- `-cb=contrast,bright` : Contraste/luminosité (contrast: -100 à 100, bright: -100 à 100)
- `-s=nb,min,max` : Taches d'encre (nombre, rayon minimum, rayon maximum)
- `-r=angle` : Rotation en degrés
- `-t=dx,dy` : Translation (déplacement horizontal, déplacement vertical)
- `-seed=n` : Initialisation du générateur aléatoire
- `-coef=n` : Coefficient global de distorsion (0 à 1)

### Utilisation dans les benchmarks

Le module est intégré au benchmark `gen-parse` qui évalue automatiquement la robustesse des marqueurs face à ces perturbations. La fonction `random_exec()` génère une combinaison aléatoire de ces effets, créant un environnement de test réaliste.

Vous pouvez contrôler le comportement du module via l'option `--seed` :

```sh
# Utilisation avec une graine spécifique pour des résultats reproductibles
./build-cmake/bench --benchmark gen-parse --nb-copies 5 --seed 12345

# Utilisation avec une graine aléatoire (basée sur le temps) pour plus de variété
./build-cmake/bench --benchmark gen-parse --nb-copies 5 --seed 0
```

## Structure du projet

```
.
├── include/                   # Fichiers d'en-tête
│   └── common.h               # Définitions de structures communes
├── src/                       # Code source C++ principal
│   ├── bench/                 # Code source des benchmarks
│   │   ├── config_analysis.cpp # Analyse des configurations
│   │   └── gen_parse.cpp      # Génération et parsing de marqueurs
│   ├── external-tools/        # Outils externes
│   │   ├── create_copy.cpp    # Création de copies d'examen
│   │   ├── create_copy.h      # Définitions pour création de copies
│   │   ├── modifier_constants.h # Constantes pour modifications
│   │   ├── modifier.cpp       # Application de modifications d'image
│   │   └── modifier.h         # Définitions pour modifications
│   ├── parser/                # Implémentations des parseurs de marqueurs
│   ├── utils/                 # Utilitaires partagés
│   ├── benchmark.cpp          # Point d'entrée pour les benchmarks
│   ├── expl_pars.cpp          # Point d'entrée pour parser des copies individuelles
│   ├── modifier_cli.cpp           # Outil de simulation de défauts d'image pour tester la robustesse des marqueurs
│   └── gen_copies.cpp    # Interface entre les arguments CLI et le générateur de copies
├── tools/                     # Scripts et utilitaires
│   ├── format.py              # Formatter de code (clang-format)
│   └── pdf_noiser/            # Outils de simulation de défauts
│       └── printer_emulator.py # Simulateur de défauts d'impression/scan en Python
├── typst/                     # Sources de templates Typst
├── stats-analysis/            # Scripts et outils d'analyse statistique
├── copies/                    # Dossier de sortie pour les copies générées
├── output/                    # Dossier de sortie pour les résultats d'analyse
├── build-cmake/               # Répertoire de build CMake (généré)
├── build/                     # Répertoire de build Meson (généré)
├── README.md                  # Ce fichier
└── LICENSE                    # Fichier de licence
```

### Architecture du projet et guide d'extension

#### Vue d'ensemble de l'architecture

Le projet est organisé selon une architecture modulaire qui facilite l'ajout de nouvelles fonctionnalités:

1. **Système de génération de copies** - Crée des documents avec différents types de marqueurs
2. **Système de détection/parsing** - Analyse les images pour détecter et interpréter les marqueurs
3. **Système de benchmarking** - Évalue les performances des différents marqueurs et parseurs
4. **Utilitaires partagés** - Fournit des fonctions communes utilisées par les différents modules

Le flux de données typique est le suivant:
```
[Générateur de copies] → [Copies avec marqueurs] → [Simulateur de défauts] → [Parseurs] → [Analyse de résultats]
```

#### Composants principaux

| Composant | Fichiers principaux | Description |
|-----------|---------------------|-------------|
| Définition des types de marqueurs | `external-tools/create_copy.h` | Énumération et structures définissant les types de marqueurs |
| Générateur de copies | `external-tools/create_copy.cpp`, `gen_copies.cpp` | Interface entre C++ et Typst pour générer des copies |
| Parseurs | `parser/*.cpp` | Implémentations de différentes méthodes de détection des marqueurs |
| Système de benchmark | `benchmark.cpp`, `bench/*.cpp` | Infrastructure pour évaluer et comparer les performances |
| Simulateur de défauts | `modifier.cpp`, `external-tools/modifier.h` | Fonctions pour simuler les défauts d'impression et de numérisation |
| Utilitaires | `utils/*.cpp` | Fonctions partagées pour la manipulation d'images, JSON, etc. |

#### Guide: Ajouter un nouveau type de marqueur

Pour ajouter un nouveau type de marqueur (par exemple, un marqueur en forme d'étoile), suivez ces étapes:

1. **Mettre à jour l'énumération `MarkerType`** dans `external-tools/create_copy.h`:

   ```cpp
   enum class MarkerType {
       // Types existants...
       STAR,        // Ajoutez votre nouveau type ici
       NONE
   };
   ```

2. **Ajouter le type aux unordered_maps de conversion** dans `external-tools/create_copy.cpp`:

   ```cpp
   const std::unordered_map<MarkerType, std::string> markerTypeToString = {
       // Mappings existants...
       { MarkerType::STAR, "star" },
   };

   const std::unordered_map<std::string, MarkerType> stringToMarkerType = {
       // Mappings existants...
       { "star", MarkerType::STAR },
   };
   ```

3. **Créer un composant Typst** pour votre marqueur dans `typst/components/star.typ`:

   ```typ
   #let star_marker(size: 10pt, outlined: false, content: none, fill_color: black) = {
     // Implémentation du marqueur en
   }
   ```

4. **Mettre à jour le système de sélection des marqueurs** dans `typst/components/corner_markers.typ`

#### Guide: Implémenter un nouveau parseur

Pour créer un nouveau parseur (par exemple, pour détecter des marqueurs en forme d'étoile), procédez comme suit:

1. **Créer les fichiers d'en-tête et d'implémentation** dans `src/parser/`:

   ```cpp
   // star_parser.h
   #ifndef STAR_PARSER_H
   #define STAR_PARSER_H

   std::optional<cv::Mat> star_parser(const cv::Mat& img,
   #ifdef DEBUG
                                       cv::Mat debug_img,
   #endif
                                       Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode);

   #endif // STAR_PARSER_H

   // star_parser.cpp
   #include <common.h>
   // Implémentez la fonction de détection de votre marqueur
   ```

2. **Mettre à jour l'énumération `ParserType`** dans `include/common.h`:

   ```cpp
   enum class ParserType { 
       // Types existants...
       STAR,
       // ...
   };
   ```

3. **Enregistrer le nouveau parseur** dans `utils/parser_helper.cpp`:

   ```cpp
   // Ajouter votre parser à la fonction string_to_parser_type
   ParserType string_to_parser_type(const std::string& parser_type_str) {
       // Parseurs existants...
       if (parser_type_str == "STAR") return ParserType::STAR;
       // ...
   }

   // Et à la fonction run_parser
   std::optional<cv::Mat> run_parser(ParserType parser_type, const cv::Mat& img,
   #ifdef DEBUG
                                     cv::Mat& debug_img,
   #endif
                                     Metadata& meta, std::vector<cv::Point2f>& dst_corner_points, int flag_barcode) {
       switch (parser_type) {
           // Cas existants...
           case ParserType::STAR:
               return star_parser(img, 
   #ifdef DEBUG
                                  debug_img,
   #endif
                                  meta, dst_corner_points, flag_barcode);
           // ...
       }
   }
   ```

#### Guide: Ajouter un nouveau benchmark

Pour créer un nouveau benchmark (par exemple, pour évaluer la robustesse aux taches d'encre):

1. **Créer les fichiers d'en-tête et d'implémentation** dans `src/bench/`:

   ```cpp
   // ink_stain_robustness.h
   #ifndef INK_STAIN_ROBUSTNESS_H
   #define INK_STAIN_ROBUSTNESS_H

   void ink_stain_robustness_benchmark(const std::unordered_map<std::string, Config>& config);

   #endif

   // ink_stain_robustness.cpp
   // Implémentation du benchmark
   ```

2. **Ajouter le benchmark à l'unordered_map** dans `benchmark.cpp`:

   ```cpp
   std::vector<std::pair<std::string, Config>> ink_stain_config = {
       // Configuration spécifique à ce benchmark
   };

   std::unordered_map<std::string, BenchmarkConfig> benchmark_map = {
       // Benchmarks existants...
       { "ink-stain-robustness", { "Ink Stain Robustness Benchmark", 
                                   ink_stain_robustness_benchmark, 
                                   ink_stain_config } },
   };
   ```

## Architecture Typst

Typst est un système moderne de composition de documents utilisé dans ce projet pour générer les copies d'examen avec les différents types de marqueurs. Voici la structure du répertoire typst/ telle qu'organisée dans le projet :

```
typst/
├── assets/                  # Ressources graphiques pour les marqueurs
│   ├── 4x4_1000-*.svg       # Marqueurs ArUco
│   ├── cross.svg            # Marqueur en forme de croix
│   ├── marker-custom.svg    # Marqueur personnalisé
│   └── qr_eye.svg           # Marqueur en forme d'œil de QR code
├── common/                  # Utilitaires et variables partagées
│   ├── global_variables.typ # Variables d'état globales
│   └── utils.typ            # Fonctions utilitaires
├── components/              # Composants réutilisables
│   ├── barcode.typ          # Génération de codes-barres avec Tiaoma
│   ├── container.typ        # Conteneur générique avec métadonnées
│   ├── corner_markers.typ   # Placement des marqueurs dans les coins
│   └── id-consent-box.typ   # Zone de consentement/identification
├── content/                 # Contenu des copies d'examen
├── src/                     # Scripts de génération
│   └── gen_copy.typ         # Générateur de copies
├── style/                   # Configuration de style
└── template.typ             # Template principal appelé par C++
```

### Fonctionnement de la génération de documents

Le processus de génération avec Typst fonctionne comme suit :

**Entrée via template.typ** : Point d'entrée principal appelé directement par les commandes C++ via `typst compile`.

**Génération des copies** : La fonction `gen-copies` dans `src/gen_copy.typ` configure la page et place les marqueurs.

**Placement des marqueurs** : Les marqueurs sont placés dans les coins et en-tête via les fonctions dans `components/corner_markers.typ`.

**Encodage des données** : Les codes QR et autres marqueurs encodables contiennent l'ID de l'examen, le numéro de page et de copie, générés via `components/barcode.typ`.

**Métadonnées JSON** : Pendant la génération, les positions des marqueurs et des boîtes utilisateurs sont enregistrées dans un état `atomic-boxes` qui est ensuite exporté au format JSON via la balise `<atomic-boxes>` pour être utilisé par le système de parsing.

### Génération et utilisation des contenus d'examen

Le système inclut également un mécanisme sophistiqué pour générer le contenu pédagogique des copies d'examen :

**Contenus modulaires** : Le contenu est organisé en modules distincts dans le répertoire `content/` :
- `content.typ` : Contenu standard d'examen
- `presentation-grid.typ` : Grilles de présentation structurées
- `understanding.typ` : Questions de compréhension

**Sélection aléatoire du contenu** : Lors de la génération, la fonction `get-random-content()` dans `src/gen_copy.typ` utilise le paramètre `seed` pour sélectionner aléatoirement un type de contenu parmi les modules disponibles :

```typ
let blocks = (
  content,
  understanding-1,
  understanding-2,
  presentation-grid-1,
  presentation-grid-2
)
```

**Positionnement avec marges** : Le contenu est encapsulé dans la fonction `content-with-margins()` qui applique les marges appropriées pour éviter les chevauchements avec les marqueurs placés dans les coins.

**Contrôle via paramètres** : Le paramètre `should-generate-content` permet de générer des copies avec ou sans contenu. Ce mode "sans contenu" est particulièrement utile pour les tests et benchmarks qui se concentrent uniquement sur les marqueurs.

## Références techniques

- **OpenCV** : [https://opencv.org/](https://opencv.org/)
- **Typst** : [https://typst.app/](https://typst.app/)
- **ZXing** : [https://github.com/zxing/zxing](https://github.com/zxing/zxing)

## License

- Code: Apache-2.0
- Everything else, in particular documentation and measurements: CC-BY-SA-4.0