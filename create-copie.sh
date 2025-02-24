typst compile --root . test/copie/original_copy.typ ./copies/original.png --input nb-copies=1 --input exam-id="test1" --ppi 200 --format png
typst query --one --field value --root ./ test/copie/original_copy.typ '<atomic-boxes>' --input nb-copies=1 --input exam-id="test1" --pretty > original_boxes.json
typst compile --root . test/copie/circle_copy.typ ./copies/circle.png --input nb-copies=1 --input exam-id="test1" --ppi 200 --format png
typst query --one --field value --root ./ test/copie/circle_copy.typ '<atomic-boxes>' --input nb-copies=1 --input exam-id="test1" --pretty > circle_boxes.json