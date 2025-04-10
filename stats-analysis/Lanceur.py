import subprocess
import os

def lancer_script_r(script_path, csv_path):
    """
    Exécute un script R avec un fichier CSV en argument.
    Affiche la sortie ou les erreurs.
    """
    if not os.path.exists(script_path):
        print(f"[ERREUR] Le script R '{script_path}' n'existe pas.")
        return

    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier CSV '{csv_path}' est introuvable.")
        return

    print(f" Lancement de '{script_path}' avec '{csv_path}'...")

    try:
        result = subprocess.run(
            ["Rscript", script_path, csv_path],
            capture_output=True,
            text=True,
            check=True
        )
        print(f"Succès : {script_path}")
        if result.stdout:
            print(f"[Sortie R] {result.stdout.strip()}")
    except subprocess.CalledProcessError as e:
        print(f"Échec : {script_path}")
        print(f"[Erreur R] {e.stderr.strip()}")


if __name__ == "__main__":
    # Liste des scripts R avec leur fichier CSV associé
    scripts_a_lancer = [
        ("bruit.r", "benchmark_bruit.csv"),
        ("graph_ppi_vs_temps.r", "benchmark_bruit.csv"),
        ("figures.r", "benchmark_performance.csv"),
    ]

    # Exécution de tous les scripts
    for script, csv in scripts_a_lancer:
        lancer_script_r(script, csv)
