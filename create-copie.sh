rm -r ./copies/*
rm -r ./output/*
mkdir -p ./copies
mkdir -p ./output

# Options communes aux deux commandes
args=(--input barcode-type="qrcode" --input barcode-height=10 --input circle-diameter=3 --input nb-copies=1 --input exam-id="test1")
doc="copy_4_barcodes.typ"
root="."

# Commande de compilation
typst compile --root "$root" "typst/$doc" "./copies/copy.png" "${args[@]}" --ppi 200 --format png

# Commande de requête
typst query --one --field value --root "$root" "typst/$doc" '<atomic-boxes>' "${args[@]}" --pretty > original_boxes.json
typst query --one --field value --root "$root" "typst/$doc" '<page>' "${args[@]}" --pretty > page.json

# Commande de parsing
# ./build-cmake/parser output/ original_boxes.json copies/copy.png