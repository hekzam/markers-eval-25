#import "../components/markers/tiaoma_gen_marker.typ": generate-marker
#import "../common/utils.typ": get-indexes
#import "../common/global_variables.typ": copy-counter, generating-content
#import "../style/const.typ": MARGIN_Y

#let EXAM-CONTENT-HASH = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4="

// ----------------------------------------------------------------
// Génère un en-tête de page avec des marqueurs.
// - exam-id: Identifiant unique de l'examen.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// ----------------------------------------------------------------
#let make-header = (exam-id, marker-type, marker-height) => {
  set align(top)
  context if generating-content.get() {
    v(MARGIN_Y)
    grid(
      columns: 2,
      column-gutter: 1fr,
      {
        generate-marker("tl", ("hztl", exam-id), marker-height, type: marker-type)
      },
      {
        generate-marker("tr", ("hztr", exam-id), marker-height, type: marker-type)
      },
    )
  } else []
}

// ----------------------------------------------------------------
// Génère un pied de page avec des marqueurs.
// - exam-id: Identifiant unique de l'examen.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// ----------------------------------------------------------------
#let make-footer = (exam-id, marker-type, marker-height) => {
  set align(bottom)
  context if generating-content.get() {
    grid(
      columns: 3,
      column-gutter: 1fr,
      {
        let (copy-i, page-i) = get-indexes()
        generate-marker("bl", ("hzbl", str(copy-i), str(page-i), exam-id), marker-height, type: marker-type)
      },
      {
        grid(
          columns: 1,
          row-gutter: 2mm,
          align: center,
          [
            #exam-id #copy-counter.display()
          ],
          [
            page #counter(page).display() / #context counter(page).final().at(0)
          ],
        )
      },
      {
        let (copy-i, page-i) = get-indexes()
        generate-marker("br", ("hzbr", str(copy-i), str(page-i), EXAM-CONTENT-HASH, exam-id), marker-height, type: marker-type)
      },
    )
    v(MARGIN_Y)
  } else []
}