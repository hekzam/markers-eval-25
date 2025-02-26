#import "style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "common/global_variables.typ": copy-counter, generating-content
#import "common/utils.typ": check-type, get-indexes
#import "common/gen_copy_common.typ": process-copies-with-pagination
#import "components/markers/tiaoma_gen_barcode.typ": barcode
#import "components/markers/circle.typ": circle-box

#let EXAM-CONTENT-HASH = "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4="

// ----------------------------------------------------------------
// Génère un en-tête de page avec des cercles.
// - circle-diameter: Diamètre du cercle.
// ----------------------------------------------------------------
#let header = (circle-diameter, exam-id, barcode-type, barcode-height) => {
  set align(top)
  context if generating-content.get() {
    v(MARGIN_Y)
    grid(
      columns: 3,
      column-gutter: 1fr,
      {
        circle-box("hztl", circle-diameter)
      },
      {
        let (copy-i, page-i) = get-indexes()
        barcode("tc", ("hztc", exam-id, str(copy-i), str(page-i), EXAM-CONTENT-HASH), barcode-height, barcode-height, type: barcode-type)
      },
      {
        circle-box("hztr", circle-diameter)
      }
    )
  } else []
}

// ----------------------------------------------------------------
// Génère un pied de page avec un cercle à gauche et un barcode à droite.
// - circle-diameter: Diamètre du cercle.
// - exam-id: Identifiant unique de l'examen.
// - barcode-type: Type de barcode à utiliser.
// - barcode-height: Hauteur du barcode.
// ----------------------------------------------------------------
#let footer = (circle-diameter, exam-id) => {
  set align(bottom)
  context if generating-content.get() {
    grid(
      columns: 3,
      column-gutter: 1fr,
      {
        circle-box("hzbl", circle-diameter)
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
        circle-box("hzbl", circle-diameter)
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
// - barcode-type: Type de barcode à utiliser.
// - barcode-height: Hauteur du barcode.
// - barcode-content-gutter: Marge entre le barcode et le contenu.
// ----------------------------------------------------------------
#let gen-copies(
  exam-id, nb-copies: 1, duplex-printing: true,
  circle-diameter: 2mm,
  barcode-type: "qrcode", barcode-height: 6mm, barcode-content-gutter: 2.5mm
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(nb-copies, int, "nb-copies must be an integer")
  check-type(duplex-printing, bool, "duplex-printing must be a boolean")
  check-type(circle-diameter, length, "circle-diameter must be a length")
  check-type(barcode-type, str, "barcode-type must be a string")
  check-type(barcode-height, length, "barcode-height must be a length")
  check-type(barcode-content-gutter, length, "barcode-content-gutter must be a length")

  set page(
    width: PAGE_WIDTH,
    height: PAGE_HEIGHT,
    margin: (
      x: MARGIN_X,
      y: MARGIN_Y + barcode-height + barcode-content-gutter,
    ),
    header-ascent: 0mm,
    header: { header(circle-diameter, exam-id, barcode-type, barcode-height) },
    footer-descent: 0mm,
    footer: { footer(circle-diameter, exam-id) },
  )

  process-copies-with-pagination(nb-copies, duplex-printing)
}

// Paramètres utilisateur spécifiés en entrée.
#let b-t = sys.inputs.at("barcode-type", default: "qrcode")
#let b-h = float(sys.inputs.at("barcode-height", default: "10")) * 1mm
#let c-d = float(sys.inputs.at("circle-diameter", default: "6")) * 1mm
#let n-c = int(sys.inputs.at("nb-copies", default: "1"))
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1

// Génère les copies
#gen-copies("basic", nb-copies: n-c, duplex-printing: d-p, circle-diameter: c-d, barcode-type: b-t, barcode-height: b-h)