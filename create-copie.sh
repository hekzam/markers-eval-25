# Créer les répertoires s'ils n'existent pas
mkdir -p ./copies
mkdir -p ./output

# Nettoyer les répertoires existants
rm -rf ./copies/*
rm -rf ./output/*

# Paramètres globaux
encoded_marker_size="15"
fiducial_marker_size="10"
marker_margin="3"
nb_copies="1"
duplex_printing="0"
marker_config="2"

# Config 1: QR codes avec données encodées dans tous les coins
# Config 2: QR codes avec données encodées uniquement dans le coin bas-droit
# Config 3: Cercles dans les trois premiers coins, QR code avec données dans le coin bas-droit
# Config 4: Cercles en haut, rien en bas-gauche, QR code avec données en bas-droit
# Config 5: Marqueurs SVG personnalisés dans trois coins, QR code avec données en bas-droit
# Config 6: Différents marqueurs ArUco, QR code avec données en bas-droit
# Config 7: Deux marqueurs ArUco, rien en bas-gauche, QR code avec données en bas-droit

while [[ $# -gt 0 ]]; do
  case $1 in
    --encoded-marker-size=*)
      encoded_marker_size="${1#*=}"
      shift
      ;;
    --fiducial-marker-size=*)
      fiducial_marker_size="${1#*=}"
      shift
      ;;
    --marker-margin=*)
      marker_margin="${1#*=}"
      shift
      ;;
    --nb-copies=*)
      nb_copies="${1#*=}"
      shift
      ;;
    --duplex-printing=*)
      duplex_printing="${1#*=}"
      shift
      ;;
    --marker-config=*)
      marker_config="${1#*=}"
      shift
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
  esac
done

# Options communes aux deux commandes
doc="template.typ"
root="."

params=(
  "--input" "encoded-marker-size=$encoded_marker_size"
  "--input" "fiducial-marker-size=$fiducial_marker_size"
  "--input" "marker-margin=$marker_margin"
  "--input" "nb-copies=$nb_copies"
  "--input" "duplex-printing=$duplex_printing"
  "--input" "marker-config=$marker_config"
)

# Commande de compilation
typst compile --root "$root" "${params[@]}" "typst/$doc" "./copies/copy.png" --format png

# Commande de requête
typst query --one --field value --root "$root" "${params[@]}" "typst/$doc" '<atomic-boxes>' --pretty > original_boxes.json
typst query --one --field value --root "$root" "${params[@]}" "typst/$doc" '<page>' --pretty > page.json
# Commande de parsing
# ./build-cmake/parser output/ original_boxes.json copies/copy.png