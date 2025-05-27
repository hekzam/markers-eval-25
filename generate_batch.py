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

random.seed(42)

config: list[list[str]] = json.load(open("all_config.json"))

set_config = set()
for c, _ in config:
    set_config.add(c)

print("Number of configurations:", len(set_config))
print("Number of configurations with parser:", len(config))

nb_copies_per_config = 100  # Nombre de copies pour chaque configuration
header_marker_size = 15    # Taille du marqueur d'en-tête
unencoded_marker_size = 12  # Taille du marqueur non encodé
shape_marker_size = 6       # Taille du marqueur de forme
fiducial_marker_size = 8    # Taille du marqueur fiduciaire
encoded_marker_size = 18   # Taille du marqueur encodé
dpi = 200                  # Résolution en DPI pour la génération
line = []                  # Liste pour stocker les lignes de commande

def get_marker_size(config_str):
    """Determine marker size based on configuration content"""
    if any(marker in config_str for marker in ["circle", "square", "cross"]):
        return shape_marker_size
    elif any(marker in config_str for marker in ["aruco", "qreye", "custom"]):
        return fiducial_marker_size
    else:
        return unencoded_marker_size

gen_parse_bench = False     # Drapeau pour activer/désactiver le benchmark gen-parse

if gen_parse_bench:
    all_batch = []
    # Création de plusieurs copies des configurations avec mélange pour randomisation
    for i in range(nb_copies_per_config):
        all_batch += config

    first = True

    for config_, parser in all_batch:
        marker_size = get_marker_size(config_)
        line.append(f"gen-parse --nb-copies 1 {"--warmup-iterations 1" if first else ""} --seed {random.randint(0,10000)} --parser-type {parser} --header-marker-size {header_marker_size} --marker-config {config_} --unencoded-marker-size {marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename gen-parse.csv --csv-mode append\n")
        first = False

config_analysis_bench = True
if config_analysis_bench:
    for config_ in set_config:
        marker_size = get_marker_size(config_)
        line.append(f"config-analysis --header-marker-size {header_marker_size} --marker-config {config_} --unencoded-marker-size {marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename config-analysis.csv --csv-mode append\n")

limite_bench = False
if limite_bench:
    all_batch = []
    for i in range(1):
        all_batch += config
    
    first = True
    for config_, parser in all_batch:
        marker_size = get_marker_size(config_)
        line.append(f"limite-bench --nb-copies 1 {"--warmup-iterations 1" if first else ""} --seed {random.randint(0,10000)} --parser-type {parser} --header-marker-size {header_marker_size} --marker-config {config_} --unencoded-marker-size {marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename limite.csv --csv-mode append\n")
        first = False

random_bench = True
if random_bench:
    random.shuffle(line)

with open("batch.txt", "w") as f:
    f.write("".join(line))
    f.write("\n")