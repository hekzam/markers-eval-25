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

## Exécution du programme

Une fois la compilation terminée, utilisez la commande suivante pour générer les copies :

```sh
./create-copie.sh [options]
```

Ce script permet de produire une copie vers le dossier de sortie **copies/**.

### Options disponibles

```
  --encoded-size N      : Taille des marqueurs encodés (par défaut: 15)
  --fiducial-size N     : Taille des marqueurs fiduciaires (par défaut: 10)
  --stroke-width N      : Largeur du trait des marqueurs (par défaut: 2)
  --margin N            : Marge autour des marqueurs (par défaut: 3)
  --duplex N            : Mode d'impression recto-verso (0: simple face, 1: recto-verso) (par défaut: 0)
  --config N            : Configuration des marqueurs (1-10) (par défaut: 10)
  --grey-level N        : Niveau de gris (0: noir, 255: blanc) (par défaut: 100)
  --header-marker N     : Affiche un marqueur d'entête (par défaut: 1)
  --filename NAME       : Nom du fichier de sortie (par défaut: copy)
```

Exemple d'utilisation :
```sh
./create-copie.sh --config 3 --grey-level 50
```

Une autre exemple avec un nom de fichier personnalisé :
```sh
./create-copie.sh --config 3 --grey-level 50 --filename exam01
```

### Configurations de marqueurs disponibles

Le paramètre `--config` permet de sélectionner parmi les configurations suivantes :

1.  : QR codes avec données encodées dans tous les coins
2.  : QR codes avec données encodées uniquement dans le coin bas-droit
3.  : Cercles dans les trois premiers coins, QR code avec données dans le coin bas-droit
4.  : Cercles en haut, rien en bas-gauche, QR code avec données en bas-droit
5.  : Marqueurs SVG personnalisés dans trois coins, QR code avec données en bas-droit
6.  : Différents marqueurs ArUco, QR code avec données en bas-droit
7.  : Deux marqueurs ArUco, rien en bas-gauche, QR code avec données en bas-droit
8.  : Cercles non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
9.  : Carrés dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit
10. : Carrés non remplis dans les trois premiers coins, QR code avec données encodées dans le coin bas-droit

## 📂 Structure du projet

```
.
├── include/            # Fichiers d'en-tête (*.h, *.hpp)
│   ├── benchmark.hpp   # En-têtes pour le benchmarking
│   └── common.h        # Définitions de structures communes
├── src/                # Code source C++ principal
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
├── copies/             # Dossier de sortie pour les copies générées
├── output/             # Dossier de sortie pour les résultats d'analyse
├── build-cmake/        # Répertoire de build (généré)
├── CMakeLists.txt      # Configuration du projet CMake
├── create-copie.sh     # Script de génération de copies
└── README.md           # Ce fichier
```

## 📖 Références techniques

- **OpenCV** : [https://opencv.org/](https://opencv.org/)
- **Typst** : [https://typst.app/](https://typst.app/)
- **ZXing** : [https://github.com/zxing/zxing](https://github.com/zxing/zxing)

## License

- Code: Apache-2.0
- Everything else, in particular documentation and measurements: CC-BY-SA-4.0
