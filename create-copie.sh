rm -rf ./copies/*
rm -rf ./output/*
typst compile --root . test/copie/content.typ ./copies/original.png --input nb-copies=1 --input exam-id="test1" --ppi 200 --format png
typst query --one --field value --root ./ test/copie/content.typ '<atomic-boxes>' --input nb-copies=1 --input exam-id="test1" --pretty > original_boxes.json
./build-cmake/parser output/ original_boxes.json copies/original.png