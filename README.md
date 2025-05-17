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

## Génération de copie

Une fois la compilation terminée, vous pouvez générer des copies d'examen avec différents types de marqueurs.

Utilisez la commande suivante pour générer des copies avec des options personnalisées :

```sh
./create-copie.sh [options]
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
./create-copie.sh --tl qrcode --tr qrcode --bl qrcode --br qrcode --header qrcode
```

#### Configuration avancée avec différents marqueurs
```sh
./create-copie.sh --tl circle:outlined --tr circle:outlined --bl none --br qrcode:encoded --header qrcode:encoded --encoded-size 20 --unencoded-size 12 --grey-level 80 --header-size 18 --content-margin-x 15 --content-margin-y 25 --seed 123 --dpi 600 --filename exam_high_res
```

## Exécution du benchmark

Le projet inclut plusieurs outils de benchmark pour évaluer différents aspects des marqueurs, comme leur consommation d'encre, leur facilité de détection, et leurs performances globales.

### Exécution des benchmarks

Vous disposez de deux méthodes pour exécuter les benchmarks :

#### 1. Mode ligne de commande (recommandé)

Spécifiez directement tous les paramètres dans votre commande :

```sh
./run_benchmark.sh --benchmark [nom-du-benchmark] [autres-options]
```

Exemple :
```sh
./run_benchmark.sh --benchmark gen-parse --nb-copies 5 --marker-config "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)" --parser-type QRCODE
```

#### 2. Mode interactif

Exécutez simplement la commande en spécifiant au minimum le type de benchmark :

```sh
./run_benchmark.sh --benchmark [nom-du-benchmark]
```

Le script vous guidera ensuite pour saisir les autres paramètres via une interface interactive dans le terminal.

> **Note** : Si vous ne spécifiez pas de type avec l'option `--benchmark`, le benchmark par défaut sera `config-analysis`.

#### 3. Mode batch

Vous pouvez également exécuter une série de benchmarks depuis un fichier texte contenant les commandes :

```sh
./run_benchmark.sh --batch-file [chemin-vers-fichier]
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
   ./run_benchmark.sh --benchmark gen-parse
   ```

   **Options spécifiques** :
   - `--parser-type <type>` : Type de parseur à utiliser (QRCODE, ARUCO, CIRCLE, etc.)
   - `--nb-copies <N>` : Nombre de copies à générer et analyser
   - `--seed <N>` : Graine pour la génération aléatoire (0 pour une graine basée sur le temps)
   - `--warmup-iterations <N>` : Nombre d'itérations d'échauffement avant les mesures

2. **config-analysis** : Analyse la consommation d'encre et la surface occupée par les marqueurs.
   ```sh
   ./run_benchmark.sh --benchmark config-analysis
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

6. **CUSTOM_MARKER** : Détecte des formes personnalisées définies comme marqueurs. Expérimental.

7. **SHAPE** : Détecte les marqueurs basés sur des formes géométriques simples. Utilise un processus de détection des contours.

8. **DEFAULT_PARSER** : Implémentation par défaut (ne fait rien). Utile principalement à des fins de test ou comme point de départ pour de nouveaux parseurs.

Exemple d'utilisation avec un parseur spécifique :
```sh
./run_benchmark.sh --benchmark gen-parse --parser-type CIRCLE --marker-config "(circle:outlined,circle:outlined,circle:outlined,circle:outlined,none)"
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
./run_benchmark.sh --benchmark gen-parse --nb-copies 3 --parser-type QRCODE
```

#### Exemple 2 : Analyse de consommation d'encre pour un marqueur spécifique
```sh
./run_benchmark.sh --benchmark config-analysis --marker-config "(circle:outlined,circle:outlined,circle:outlined,circle:outlined,none)" --grey-level 50
```

#### Exemple 3 : Benchmark complet avec options avancées
```sh
./run_benchmark.sh --benchmark gen-parse --nb-copies 10 --parser-type QRCODE --marker-config "(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded)" --encoded-marker-size 15 --warmup-iterations 2 --seed 42
```

## Simulateur de scan et d'impression

Le projet inclut un simulateur Python qui permet d'appliquer diverses transformations aux documents générés, simulant ainsi des défauts d'impression et de numérisation pour des tests de robustesse.

### Exécution du simulateur

Pour exécuter le simulateur sur une image originale, utilisez la commande suivante :

```sh
python tools/pdf_noiser/printer_emulator.py [options]
```

Par défaut, le script appliquera des transformations aléatoires à l'image originale située dans `copies/original.png` et générera 10 copies avec des défauts différents dans le dossier `tools/pdf_noiser/noisy_copies/`.

### Transformations disponibles

Le simulateur peut appliquer les transformations suivantes pour imiter les défauts d'impression et de numérisation :

- **Rotation** (`-r`, `--rotation`) : Applique une rotation à l'image (en degrés, ±3° max par défaut)
- **Translation** (`-t`, `--translation`) : Déplace l'image (déplacement X,Y, ±25px max par défaut)
- **Contraste** (`-c`, `--contrast`) : Modifie le contraste (0-100%, plage effective 0.8-1.2)
- **Luminosité** (`-b`, `--brightness`) : Ajuste la luminosité (0-100%, plage effective 0.8-1.2)
- **Bruit gaussien** (`-g`, `--gaussian`) : Ajoute du bruit gaussien (0-100%, intensité 4-6 max)
- **Bruit sel et poivre** (`-s`, `--salt_pepper`) : Ajoute des pixels noirs et blancs aléatoires (0-100%, 6% max)
- **Taches aléatoires** (`-p`, `--spot`) : Ajoute des taches noires (0-100%, 2-5 taches par défaut)
- **Nombre de copies** (`-n`, `--nb_copy`) : Nombre de copies à générer (10 par défaut)

### Exemples d'utilisation

#### Générer 5 copies avec des défauts aléatoires
```sh
python tools/pdf_noiser/printer_emulator.py --nb_copy 5
```

#### Générer une copie avec une rotation spécifique
```sh
python tools/pdf_noiser/printer_emulator.py --rotation 2 --nb_copy 1
```

#### Combiner plusieurs transformations
```sh
python tools/pdf_noiser/printer_emulator.py --rotation 1.5 --contrast 75 --brightness 60 --gaussian 30 --nb_copy 3
```

## Structure du projet

```
.
├── include/                   # Fichiers d'en-tête
│   └── common.h               # Définitions de structures communes
├── src/                       # Code source C++ principal
│   ├── bench/                 # Code source des benchmarks
│   ├── benchmark.cpp          # Outil de benchmarking
│   ├── expl_pars.cpp          # Parseur principal
│   ├── typst_interface.cpp    # Interface avec Typst
│   ├── utils/                 # Utilitaires partagés
│   ├── parser/                # Implémentation des parseurs de marqueurs
│   └── external-tools/        # Outils externes (création de copies)
├── tools/                     # Scripts et utilitaires
│   ├── format.py              # Formatter de code (clang-format)
│   └── pdf_noiser/            # Outils de simulation de défauts
│       └── printer_emulator.py # Simulateur de défauts d'impression/scan
├── typst/                     # Sources de templates Typst
│   ├── components/            # Composants réutilisables (marqueurs, conteneurs)
│   ├── common/                # Variables et utilitaires communs
│   ├── content/               # Contenu des formulaires
│   ├── src/                   # Scripts de génération
│   ├── style/                 # Configuration de style
│   └── template.typ           # Template principal
├── stats-analysis/            # Scripts et outils d'analyse statistique
├── copies/                    # Dossier de sortie pour les copies générées
├── output/                    # Dossier de sortie pour les résultats d'analyse
├── build-cmake/               # Répertoire de build (généré)
├── CMakeLists.txt             # Configuration du projet CMake
├── create-copie.sh            # Script de génération de copies
├── run_benchmark.sh           # Script d'exécution du benchmark
├── README.md                  # Ce fichier
└── LICENSE                    # Fichier de licence
```

## Références techniques

- **OpenCV** : [https://opencv.org/](https://opencv.org/)
- **Typst** : [https://typst.app/](https://typst.app/)
- **ZXing** : [https://github.com/zxing/zxing](https://github.com/zxing/zxing)

## License

- Code: Apache-2.0
- Everything else, in particular documentation and measurements: CC-BY-SA-4.0