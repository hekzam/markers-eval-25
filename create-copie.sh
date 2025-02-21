typst compile --root . test/copie/original_copy.typ ./copies/original.pdf --input nb-copies=1 --input exam-id="test1"
typst query --one --field value --root ./ test/copie/original_copy.typ '<atomic-boxes>' --input nb-copies=1 --input exam-id="test1" --pretty > original_boxes.json

typst compile --root . test/copie/circle_copy.typ ./copies/circle.pdf --input nb-copies=1 --input exam-id="test1"
typst query --one --field value --root ./ test/copie/circle_copy.typ '<atomic-boxes>' --input nb-copies=1 --input exam-id="test1" --pretty > circle_boxes.json