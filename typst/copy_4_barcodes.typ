#import "style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "common/global_variables.typ": copy-counter, generating-content
#import "common/utils.typ": check-type, get-indexes
#import "common/gen_copy_common.typ": process-copies-with-pagination
#import "components/markers/tiaoma_gen_marker.typ": generate-marker

#let EXAM-CONTENT-HASH = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4="

// ----------------------------------------------------------------
// Génère un en-tête de page avec des marqueurs.
// - exam-id: Identifiant unique de l'examen.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// ----------------------------------------------------------------
#let header = (exam-id, marker-type, marker-height) => {
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
#let footer = (exam-id, marker-type, marker-height) => {
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

// ----------------------------------------------------------------
// Génère des copies d'un contenu donné.
// - exam-id: Identifiant unique de l'examen.
// - nb-copies: Nombre de copies à générer.
// - duplex-printing: Impression recto verso.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// - marker-content-gutter: Marge entre le marqueur et le contenu.
// ----------------------------------------------------------------
#let gen-copies(
  exam-id, nb-copies: 1, duplex-printing: true,
  marker-type: "qrcode", marker-height: 6mm, marker-content-gutter: 2.5mm
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(marker-type, str, "marker-type must be a string")
  check-type(marker-height, length, "marker-height must be a length")
  check-type(marker-content-gutter, length, "marker-content-gutter must be a length")

  set page(
    width: PAGE_WIDTH,
    height: PAGE_HEIGHT,
    margin: (
      x: MARGIN_X,
      y: MARGIN_Y + marker-height + marker-content-gutter,
    ),
    header-ascent: 0mm,
    header: { header(exam-id, marker-type, marker-height) },
    footer-descent: 0mm,
    footer: { footer(exam-id, marker-type, marker-height) },
  )

  process-copies-with-pagination(nb-copies, duplex-printing)
}

// Paramètres utilisateur spécifiés en entrée.
#let marker-type = sys.inputs.at("marker-type", default: "qrcode")
#let marker-height = float(sys.inputs.at("marker-height", default: "10")) * 1mm
#let nb-copies = int(sys.inputs.at("nb-copies", default: "1"))
#let duplex-printing = int(sys.inputs.at("duplex-printing", default: "0")) == 1

// Génère les copies
#gen-copies("basic", nb-copies: nb-copies, duplex-printing: duplex-printing, marker-type: marker-type, marker-height: marker-height)