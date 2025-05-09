#!/usr/bin/env python3
import subprocess
import os
import sys
import argparse
from pathlib import Path

def lancer_script_r(script_path, csv_path):
    """
    Exécute un script R avec un fichier CSV en argument.
    Affiche la sortie ou les erreurs.
    """
    if not os.path.exists(script_path):
        print(f"[ERREUR] Le script R '{script_path}' n'existe pas.")
        return False

    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier CSV '{csv_path}' est introuvable.")
        return False

    print(f"\n======================================")
    print(f"Lancement de '{script_path}' avec '{csv_path}'...")
    print(f"======================================\n")

    try:
        result = subprocess.run(
            ["Rscript", script_path, csv_path],
            capture_output=True,
            text=True,
            check=True
        )
        print(f"Succès : {script_path}")
        if result.stdout:
            # Filtrer les lignes de sortie pour améliorer la lisibilité
            filtered_output = "\n".join([line for line in result.stdout.strip().split("\n") 
                                        if line.strip() and not line.startswith("Warning")])
            print(f"\n[Sortie R]\n{filtered_output}\n")
        
        # Vérifier si des graphiques ont été générés
        graphiques = [f for f in os.listdir() if f.endswith('.png')]
        if graphiques:
            print(f"Graphiques générés : {len(graphiques)} fichiers")
            for graph in graphiques[:5]:  # Afficher seulement les 5 premiers
                print(f"   - {graph}")
            if len(graphiques) > 5:
                print(f"   - ... et {len(graphiques) - 5} autres")
        
        return True
    except subprocess.CalledProcessError as e:
        print(f"Échec : {script_path}")
        print(f"[Erreur R]\n{e.stderr.strip()}")
        return False

def main():
    # Créer un analyseur d'arguments
    parser = argparse.ArgumentParser(description="Lance l'analyse R sur le fichier CSV.")
    parser.add_argument("--csv", help="Spécifier un fichier CSV alternatif", default="csv_by_parser.csv")
    args = parser.parse_args()
    
    # Fichier CSV principal
    csv_principal = args.csv
    
    # Vérifier si le fichier CSV existe
    if not os.path.exists(csv_principal):
        print(f"[ERREUR] Le fichier CSV principal '{csv_principal}' est introuvable.")
        return
    
    # Répertoire de ce script
    script_dir = Path(__file__).parent.resolve()

    # Script principal à lancer
    script_principal = "intra_parser_analysis.r"
    
    # Exécuter le script principal
    print("\nLANCEMENT DE L'ANALYSE")
    success = lancer_script_r(script_dir / script_principal, csv_principal)
    
    if success:
        print("\nL'analyse a été complétée avec succès!")
    else:
        print("\nL'analyse a échoué.")

if __name__ == "__main__":
    main()