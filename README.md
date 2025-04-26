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

Une fois la compilation terminée, vous pouvez générer des copies d'examen avec différents types de marqueurs.

### Méthodes de génération

#### 1. Mode ligne de commande

Utilisez la commande suivante pour générer des copies avec des options personnalisées :

```sh
./create-copie.sh [options]
```

Les copies générées sont sauvegardées dans le dossier **copies/**.

#### 2. Configuration rapide

Si vous ne spécifiez pas d'options, les valeurs par défaut seront utilisées :

```sh
./create-copie.sh
```

Ceci génère une copie avec des marqueurs QR code et les paramètres par défaut.

### Types de marqueurs disponibles

| Encodable       | Non encodable     | Rectangulaire    |
|-----------------|-------------------|------------------|
| qrcode          | circle            | pdf417           |
| microqr        | square            | rmqr             |
| datamatrix      | triangle          | code128          |
| aztec           | cross             |                  |
| pdf417          | aruco             |                  |
| rmqr            | qreye            |                  |
| code128         | custom            |                  |

### Options de configuration

- `--encoded-size <N>`          : Taille des marqueurs encodés (par défaut: 15)
- `--unencoded-size <N>`        : Taille des marqueurs non encodés (par défaut: 3)
- `--header-size <N>`           : Taille du marqueur d'entête (par défaut: 7)
- `--stroke-width <N>`          : Largeur du trait des marqueurs (par défaut: 2)
- `--margin <N>`                : Marge autour des marqueurs (par défaut: 3)
- `--grey-level <N>`            : Niveau de gris (0: noir, 255: blanc) (par défaut: 0)
- `--dpi <N>`                   : Résolution en points par pouce (par défaut: 300)
- `--generating-content <BOOL>` : Générer le contenu dans le document (1/true ou 0/false) (par défaut: 1)
- `--filename <name>`           : Nom du fichier de sortie (par défaut: copy)
- `--tl <type>`                 : Type de marqueur pour le coin supérieur gauche
- `--tr <type>`                 : Type de marqueur pour le coin supérieur droit
- `--bl <type>`                 : Type de marqueur pour le coin inférieur gauche
- `--br <type>`                 : Type de marqueur pour le coin inférieur droit
- `--header <type>`             : Type de marqueur pour l'en-tête

#### Format des types de marqueurs

```
  Format: type[:encoded][:outlined]
  - type:outlined     : Marqueur non rempli (uniquement pour formes géométriques simples)
  - type:encoded      : Marqueur avec données encodées
  - type:unencoded    : Marqueur sans données encodées
```

### Exemples

#### Exemple simple avec des QR codes
```sh
./create-copie.sh --tl qrcode --tr qrcode --bl qrcode --br qrcode --header qrcode
```

#### Configuration avancée avec différents marqueurs
```sh
./create-copie.sh --tl circle:outlined --tr circle:outlined --bl none --br qrcode:encoded --header qrcode:encoded --encoded-size 20 --unencoded-size 12 --grey-level 80 --header-size 18 --dpi 600 --filename exam_high_res
```

## 📊 Exécution du benchmark

### Exécution des benchmarks

Vous disposez de deux méthodes pour exécuter les benchmarks :

#### 1. Mode ligne de commande (recommandé)

Spécifiez directement tous les paramètres dans votre commande :

```sh
./run_benchmark.sh --benchmark [nom-du-benchmark] [autres-options]
```

Exemple :
```sh
./run_benchmark.sh --benchmark parsing-time --input-dir ./copies --dpi 600
```

#### 2. Mode interactif

Exécutez simplement la commande en spécifiant au minimum le type de benchmark :

```sh
./run_benchmark.sh --benchmark [nom-du-benchmark]
```

Le script vous guidera ensuite pour saisir les autres paramètres via une interface interactive dans le terminal.

> **Note** : Si vous ne spécifiez pas de type avec l'option `--benchmark`, le benchmark par défaut sera `parsing-time`.

#### Types de benchmark disponibles

Voici les différents types de benchmarks que vous pouvez exécuter :

1. **parsing-time** : Évalue le temps de traitement et le taux de succès de la détection des marqueurs.
   ```sh
   ./run_benchmark.sh --benchmark parsing-time
   ```

2. **generation-time** : Mesure le temps nécessaire pour générer des copies avec différents types de marqueurs.
   ```sh
   ./run_benchmark.sh --benchmark generation-time
   ```

3. **ink-estimation** : Analyse la consommation d'encre pour chaque type de marqueur et fournit :
   - La surface totale couverte en cm²
   - Le pourcentage de couverture d'encre
   - Le volume d'encre estimé en millilitres
   ```sh
   ./run_benchmark.sh --benchmark ink-estimation
   ```

### Options communes

- `--benchmark <type>`          : Type de benchmark à exécuter (par défaut: `parsing-time`)
- `--marker-config <config>`    : Fichier de configuration des marqueurs (par défaut: `(qrcode:encoded,qrcode:encoded,qrcode:encoded,qrcode:encoded,none)`)
- `--encoded-marker_size <N>`   : Taille des marqueurs encodés en mm (par défaut: 13)
- `--unencoded-marker_size <N>` : Taille des marqueurs non encodés en mm (par défaut: 10)
- `--header-marker_size <N>`    : Taille du marqueur d'en-tête en mm (par défaut: 7)
- `--grey-level <0-255>`        : Niveau de gris pour les marqueurs (par défaut: 0)
- `--dpi <N>`                   : Résolution en points par pouce (par défaut: 300)

Options spécifiques pour les benchmarks `parsing-time` et `generation-time` :
- `--nb-copies <N>`             : Nombre de copies à générer pour le test (par défaut: 1)
- `--warmup-iterations <N>`     : Nombre d'itérations d'échauffement avant la mesure. Cela permet d'obtenir des mesures plus précises en évitant les coûts de démarrage (par défaut: 0)

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
