#!/usr/bin/env python3
import random
import json

"""
Script de génération de commandes batch pour les benchmarks de marqueurs.

Ce script crée un fichier batch.txt contenant des commandes pour deux types de benchmarks :
1. gen-parse : Génère et analyse des marqueurs avec différentes configurations
2. config-analysis : Analyse les différentes configurations de marqueurs

Les configurations sont chargées depuis 'all_config.json'.
"""

# Fixe la graine aléatoire pour des résultats reproductibles
random.seed(42)

# Chargement des configurations depuis le fichier JSON
config: list[list[str]] = json.load(open("all_config.json"))

# Création d'un ensemble des configurations uniques
set_config = set()
for c, _ in config:
    set_config.add(c)

print("Number of configurations:", len(set_config))
print("Number of configurations with parser:", len(config))

# Paramètres pour la génération des marqueurs
nb_copies_per_config = 20  # Nombre de copies pour chaque configuration
header_marker_size = 10    # Taille du marqueur d'en-tête
unencoded_marker_size = 8  # Taille du marqueur non encodé
encoded_marker_size = 20   # Taille du marqueur encodé
dpi = 200                  # Résolution en DPI pour la génération
line = []                  # Liste pour stocker les lignes de commande
gen_parse_bench = True     # Drapeau pour activer/désactiver le benchmark gen-parse

# Génération des commandes pour le benchmark de génération et d'analyse
if gen_parse_bench:
    all_batch = []
    # Création de plusieurs copies des configurations avec mélange pour randomisation
    for i in range(nb_copies_per_config):
        random.shuffle(config)
        all_batch += config

    first = True  # Drapeau pour identifier la première commande (traitement spécial)

    # Génération d'une commande pour chaque configuration
    for config, parser in all_batch:
        line.append(f"gen-parse --nb-copies 1 {"--warmup-iterations 1" if first else ""} --seed {random.randint(0,10000)} --parser-type {parser} --header-marker-size {header_marker_size} --marker-config {config} --unencoded-marker-size {unencoded_marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename gen-parse.csv {"" if first else "--csv-mode append"}\n")
        first = False

# Drapeau pour activer/désactiver le benchmark d'analyse de configuration
config_analysis_bench = True
if config_analysis_bench:
    first = True  # Drapeau pour identifier la première commande (traitement spécial)
    # Génération d'une commande pour chaque configuration unique
    for config in set_config:
        line.append(f"config-analysis --header-marker-size {header_marker_size} --marker-config {config} --unencoded-marker-size {unencoded_marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename config-analysis.csv {"" if first else "--csv-mode append"}\n")
        first = False

# Écriture de toutes les commandes dans le fichier batch.txt
with open("batch.txt", "w") as f:
    f.write("".join(line))
    f.write("\n")