# Créer les répertoires s'ils n'existent pas
mkdir -p ./copies
mkdir -p ./output

# Nettoyer les répertoires existants
rm -rf ./copies/*
rm -rf ./output/*

# Options communes aux deux commandes
doc="template.typ"
root="."

# Commande de compilation
typst compile --root "$root" "typst/$doc" "./copies/copy.png" --format png

# Commande de requête
typst query --one --field value --root "$root" "typst/$doc" '<atomic-boxes>' "${args[@]}" --pretty > original_boxes.json
typst query --one --field value --root "$root" "typst/$doc" '<page>' --pretty > page.json
# Commande de parsing
# ./build-cmake/parser output/ original_boxes.json copies/copy.png