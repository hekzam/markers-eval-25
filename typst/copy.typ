#import "common/global_variables.typ": copy-counter
#import "style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "common/utils.typ": check-type, update-page-state, finalize-states
#import "marker-setup/4-barcodes.typ": make-header, make-footer
#import "content/content.typ": content

// ----------------------------------------------------------------
// Génère des copies d'un contenu donné.
// - exam-id: Identifiant unique de l'examen.
// - copy-content: Contenu à copier.
// - nb-copies: Nombre de copies à générer.
// - duplex-printing: Impression recto verso.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// - marker-content-gutter: Marge entre le marqueur et le contenu.
// ----------------------------------------------------------------
#let gen-copies(
  exam-id, copy-content,
  nb-copies: 1, duplex-printing: true,
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
    header: { make-header(exam-id, marker-type, marker-height) },
    footer-descent: 0mm,
    footer: { make-footer(exam-id, marker-type, marker-height) },
  )

  update-page-state(PAGE_WIDTH, PAGE_HEIGHT)

  let pagebreak_to = if duplex-printing { "odd" } else { none }
  let copy-index = 0
  while copy-index < nb-copies {
    copy-counter.update(copy-index)
    copy-index = copy-index + 1
    counter(page).update(1)

    copy-content

    if copy-index < nb-copies {
      pagebreak(to: pagebreak_to)
    }
  }
  finalize-states()
}

// Paramètres utilisateur spécifiés en entrée.
#let marker-type = sys.inputs.at("marker-type", default: "qrcode")
#let marker-height = float(sys.inputs.at("marker-height", default: "10")) * 1mm
#let nb-copies = int(sys.inputs.at("nb-copies", default: "1"))
#let duplex-printing = int(sys.inputs.at("duplex-printing", default: "0")) == 1

// Génère les copies
#gen-copies("basic", content, nb-copies: nb-copies, duplex-printing: duplex-printing, marker-type: marker-type, marker-height: marker-height)