# Hekzam Markers - Optimisation des marqueurs pour la correction d'examens 📄

## 📌 Description du projet

Hekzam est un ensemble de logiciels conçu pour permettre la création et la correction automatisée ou semi-automatisée des examens papier.  
Actuellement, Hekzam utilise des **QR codes** comme marqueurs de bord pour la calibration et l'identification des pages des copies d'examen. Cependant, ces marqueurs présentent plusieurs inconvénients :

- **Coût élevé en encre** : les QR codes sont denses et nécessitent une quantité importante d'encre.
- **Espace occupé important** : ils réduisent la place disponible pour le contenu pédagogique.
- **Consommation de ressources computationnelles** : leur génération et leur reconnaissance demandent un traitement relativement lourd.

### 🎯 Objectif du projet

L'objectif de ce projet est de **développer et évaluer des alternatives aux QR codes** utilisées actuellement dans Hekzam.  
Les nouvelles solutions devront être plus sobres en encre et en espace tout en maintenant une **grande robustesse** dans leur détection.

## 🛠️ Dépendances nécessaires

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

## Exécution du programme

Une fois la compilation terminée, utilisez la commande suivante pour générer les formulaires d'examen :

```sh
./create-copie.sh
```

Ce script permet de produire des formulaires contenant les nouveaux marqueurs optimisés, tout en intégrant les métadonnées nécessaires pour leur identification et leur calibration.

## 📂 Structure du projet (to edit)

```
.
├── src/                # Code source principal (C++, OpenCV, ZXing)
├── scripts/            # Scripts utiles (build, exécution, tests)
├── docs/               # Documentation du projet
├── CMakeLists.txt      # Configuration CMake
└── README.md           # Ce fichier
```

## 📖 Références techniques

- **OpenCV** : [https://opencv.org/](https://opencv.org/)
- **Typst** : [https://typst.app/](https://typst.app/)
- **ZXing (Reconnaissance QR codes)** : [https://github.com/zxing/zxing](https://github.com/zxing/zxing)

## 🤝 Contributeurs

- 📧 **Contact** : [millian.poquet@univ-tlse3.fr](mailto:millian.poquet@univ-tlse3.fr)
- 🔬 **Université Paul Sabatier - IRIT, équipe Sepia**
- 📍 **Projet encadré dans le cadre du Bachelor Engineering**

## License

- Code: Apache-2.0
- Everything else, in particular documentation and measurements: CC-BY-SA-4.0
