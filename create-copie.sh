rm -rf ./copies/*
rm -rf ./output/*
#!/bin/bash

# Options communes aux deux commandes
common_args=(--input marker-type="aztec" --input marker-height=8 --input nb-copies=1 --input exam-id="test1")
doc="typst/copy.typ"
root="."

# Commande de compilation
typst compile --root "$root" "$doc" "./copies/copy.png" "${common_args[@]}" --ppi 200 --format png

# Commande de requête
typst query --one --field value --root "$root" "$doc" '<atomic-boxes>' "${common_args[@]}" --pretty > original_boxes.json

# ./build-cmake/parser output/ original_boxes.json copies/copy.png