#import "common/global_variables.typ": atomic-boxes, copy-counter, page-state, generating-content
#import "components/markers/tiaoma_gen_marker.typ": generate-marker
#import "content/content.typ": content
#import "common/utils.typ": check-type

// ----------------------------------------------------------------
// Finalise les états de la page et des boîtes atomiques.
// ----------------------------------------------------------------
#let finalize-states() = context [
  #metadata(atomic-boxes.final()) <atomic-boxes>
  #metadata(page-state.final()) <page>
]

// ----------------------------------------------------------------
// Génère un en-tête de page avec des marqueurs.
// Parameters:
// - exam-id: Identifiant unique de l'examen.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// - margin-y: Marge verticale.
// ----------------------------------------------------------------
#let make-header = (exam-id, marker-type, marker-height, margin-y) => {
  set align(top)
  context if generating-content.get() {
    v(margin-y)
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
// Parameters:
// - exam-id: Identifiant unique de l'examen.
// - exam-content-hash: Hash du contenu de l'examen.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// - margin-y: Marge verticale.
// ----------------------------------------------------------------
#let make-footer = (exam-id, exam-content-hash, marker-type, marker-height, margin-y) => {
  set align(bottom)
  context if generating-content.get() {
    grid(
      columns: 3,
      column-gutter: 1fr,
      {
        let copy-i = copy-counter.get().at(0)
        let page-i = counter(page).get().at(0)
        generate-marker("bl", ("hzbl", str(copy-i), str(page-i)), marker-height, type: marker-type)
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
        let copy-i = copy-counter.get().at(0)
        let page-i = counter(page).get().at(0)
        generate-marker("br", ("hzbr", str(copy-i), str(page-i), exam-content-hash, exam-id), marker-height, type: marker-type)
      },
    )
    v(margin-y)
  } else []
}

// ----------------------------------------------------------------
// Met à jour l'état de la page avec les dimensions fournies.
// Parameters:
// - width: Largeur de la page.
// - height: Hauteur de la page.
// ----------------------------------------------------------------
#let update-page-state = (width, height) => {
  page-state.update(x => {
    x.insert("width", width.mm())
    x.insert("height", height.mm())
    x
  })
}

// ----------------------------------------------------------------
// Génère des copies d'un contenu donné.
// Parameters:
// - exam-id: Identifiant unique de l'examen.
// - copy-content: Contenu à copier.
// - exam-content-hash: Hash du contenu de l'examen.
// - nb-copies: Nombre de copies à générer.
// - duplex-printing: Impression recto verso.
// - marker-type: Type de marqueur à utiliser.
// - marker-height: Hauteur du marqueur.
// - marker-content-gutter: Marge entre le marqueur et le contenu.
// - page-width: Largeur de la page.
// - page-height: Hauteur de la page.
// - margin-x: Marge horizontale.
// - margin-y: Marge verticale.
// ----------------------------------------------------------------
#let gen-copies(
  exam-id, copy-content, exam-content-hash: "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=",
  nb-copies: 1, duplex-printing: true,
  marker-type: "qrcode",
  marker-height: 6mm, marker-content-gutter: 2.5mm,
  page-width: 210mm, page-height: 297mm,
  margin-x: 10mm, margin-y: 10mm
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(page-width, length, "page-width must be a length")
  check-type(page-height, length, "page-height must be a length")
  check-type(marker-type, str, "marker-type must be a string")
  check-type(marker-height, length, "marker-height must be a length")
  check-type(margin-x, length, "margin-x must be a length")
  check-type(margin-y, length, "margin-y must be a length")
  check-type(marker-content-gutter, length, "marker-content-gutter must be a length")

  set page(
    width: page-width,
    height: page-height,
    margin: (
      x: margin-x,
      y: margin-y + marker-height + marker-content-gutter,
    ),
    header-ascent: 0mm,
    header: { make-header(exam-id, marker-type, marker-height, margin-y) },
    footer-descent: 0mm,
    footer: { make-footer(exam-id, exam-content-hash, marker-type, marker-height, margin-y) },
  )

  update-page-state(page-width, page-height)

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
#let mt = sys.inputs.at("marker-type", default: "qrcode") // Type de marqueur
#let mh = float(sys.inputs.at("marker-height", default: "10")) * 1mm // Hauteur du marqueur
#let n = int(sys.inputs.at("nb-copies", default: "1")) // Nombre de copies
#let d = int(sys.inputs.at("duplex-printing", default: "0")) == 1 // Impression recto verso

// Génère les copies
#gen-copies("basic", content, nb-copies: n, duplex-printing: d, marker-type: mt, marker-height: mh)