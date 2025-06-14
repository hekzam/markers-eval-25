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

def afficher_menu():
    """
    Affiche un menu pour choisir entre les scripts disponibles.
    """
    print("\n===== MENU DE SÉLECTION DES ANALYSES =====")
    print("1. Lancer intra_parser.r")
    print("2. Lancer inter_parser.r") 
    print("3. Lancer limite.r")
    print("4. Lancer config_analysis.r")
    print("q. Quitter")
    choix = input("\nEntrez votre choix (1, 2, 3, 4 ou q) : ")
    return choix

def main():
    # Créer un analyseur d'arguments
    parser = argparse.ArgumentParser(description="Lance l'analyse R sur le fichier CSV.")
    parser.add_argument("--csv", help="Spécifier un fichier CSV", default="../output/csv/test1.csv")
    parser.add_argument("--script", help="Spécifier le script à lancer (intra/inter/limite/config)", 
                        choices=["intra", "inter", "limite", "config"])
    parser.add_argument("--mode", help="Mode d'exécution: interactif ou direct", 
                        choices=["interactif", "direct"], default="interactif")
    args = parser.parse_args()
    
    # Fichier CSV 
    csv_principal = args.csv
    
    # Fichier CSV spécifique pour config_analysis
    csv_config = "config-analysis.csv"

    # Répertoire de ce script
    script_dir = Path(__file__).parent.resolve()
    
    # Vérifier si le fichier CSV existe
    if not os.path.exists(script_dir / csv_principal) and args.script != "config":
        print(f"[ERREUR] Le fichier CSV principal '{script_dir / csv_principal}' est introuvable.")
        return
   
    # Mode d'exécution
    if args.mode == "direct" and args.script:
        # Exécution directe si le script est spécifié
        if args.script == "intra":
            script_principal = "intra_parser_analysis.r"
            csv_file = csv_principal
        elif args.script == "inter":
            script_principal = "inter_parser_analysis.r"
            csv_file = csv_principal
        elif args.script == "config":
            script_principal = "config_analysis.r"
            csv_file = csv_config
        else:  # limite
            script_principal = "limite.r"
            csv_file = csv_principal
        
        # Exécuter le script spécifié
        print(f"\nLANCEMENT DE L'ANALYSE ({args.script})")
        success = lancer_script_r(script_dir / script_principal, script_dir / csv_file)
        
        if success:
            print("\nL'analyse a été complétée avec succès!")
        else:
            print("\nL'analyse a échoué.")
    else:
        # Mode interactif
        while True:
            choix = afficher_menu()
            
            if choix == "1":
                script_principal = "intra_parser_analysis.r"
                success = lancer_script_r(script_dir / script_principal, script_dir / csv_principal)
                
                if success:
                    print("\nL'analyse intra a été complétée avec succès!")
                else:
                    print("\nL'analyse intra a échoué.")
                
            elif choix == "2":
                script_principal = "inter_parser_analysis.r"
                success = lancer_script_r(script_dir / script_principal, script_dir / csv_principal)
                
                if success:
                    print("\nL'analyse inter a été complétée avec succès!")
                else:
                    print("\nL'analyse inter a échoué.")
                    
            elif choix == "3":
                script_principal = "limite.r"
                success = lancer_script_r(script_dir / script_principal, script_dir / csv_principal)
                
                if success:
                    print("\nL'analyse limite a été complétée avec succès!")
                else:
                    print("\nL'analyse limite a échoué.")
                
            elif choix == "4":
                script_principal = "config_analysis.r"
                success = lancer_script_r(script_dir / script_principal, script_dir / csv_config)
                
                if success:
                    print("\nL'analyse des configurations a été complétée avec succès!")
                else:
                    print("\nL'analyse des configurations a échoué.")
                
            elif choix.lower() == "q":
                print("\nFin du programme. Au revoir!")
                break
            else:
                print("\nChoix invalide. Veuillez réessayer.")

if __name__ == "__main__":
    main()