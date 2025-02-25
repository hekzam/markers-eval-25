#import "@preview/tiaoma:0.2.1"

#let atomic-boxes = state("atomic-boxes", (:)) // stockage des boîtes atomiques
#let page-state = state("page-state", (:)) // stockage des états de la page 
#let copy-counter = counter("copy-counter") // compteur de copies
#let generating-content = state("generating-content", true) // indicateur de génération de contenu

// ----------------------------------------------------------------
// Function: rect-box
// Description: Crée une boîte rectangulaire avec un identifiant unique et des propriétés spécifiques.
// Parameters:
// - id: Identifiant unique de la boîte.
// - width: Largeur de la boîte.
// - height: Hauteur de la boîte.
// - fill-color: Couleur de remplissage (valeur par défaut "white").
// - stroke-width: Épaisseur du trait (valeur par défaut 0.25mm).
// - stroke-color: Couleur du trait (valeur par défaut "black").
// - inner-content: Contenu à afficher à l'intérieur de la boîte (valeur par défaut []).
// Returns:
// - Une boîte rectangulaire avec les propriétés spécifiées.
// ----------------------------------------------------------------
#let rect-box(id, width, height, fill-color: white, stroke-width: 0.25mm, stroke-color: black, inner-content: []) = {
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-width), length, message: "stroke must be a length")

  set align(left+top)
  rect(width: width, height: height, fill: fill-color, stroke: stroke-width + stroke-color, radius: 0mm, inset: 0mm, outset: 0mm, {
    context if copy-counter.get().at(0) == 0 {
      layout(size => {
        set align(left+top)
        let pos = here().position()
        let box = (
          page: pos.page,
          x: pos.x.mm(),
          y: pos.y.mm(),
          width: if type(width) == length { width.mm() }
            else if type(width) == ratio { (width * size.width).mm() },
          height: if type(height) == length { height.mm() }
              else if type(height) == ratio { (height * size.height).mm() },
          stroke-width: stroke-width.mm(),
          stroke-color: stroke-color.to-hex(),
          fill-color: fill-color.to-hex(),
        )
        atomic-boxes.update(x => { x.insert(str(id), box) ; x })
        inner-content
      })
    } else {
      inner-content
    }
  })
}

// ----------------------------------------------------------------
// Function: finalize-states
// Description: Finalise les états de la page et des boîtes atomiques.
// ----------------------------------------------------------------
#let finalize-states() = context [
  #metadata(atomic-boxes.final()) <atomic-boxes>
  #metadata(page-state.final()) <page>
]

// ----------------------------------------------------------------
// Function: generate-marker
// Description: Génère un marker à partir des données fournies et l'encapsule dans une boîte.
// Parameters:
// - label: Un identifiant ou une étiquette pour la boîte.
// - parts: Une liste de chaînes représentant les données stockées dans le code-barre.
// - height: La hauteur (et largeur) de la boîte.
// - type: Type de marqueur (valeur par défaut "qrcode").
// Returns:
// - Une boîte contenant le marqueur généré.
// ----------------------------------------------------------------
#let generate-marker(label, parts, height, type: "qrcode") = {
  let sep = ","
  let data = parts.join(sep)
  let marker = (
    if type == "qrcode" {
      tiaoma.qrcode(data, height: height)
    } else if type == "ean13" {
      tiaoma.ean13(data, height: height)
    } else {
      error("Type de marqueur inconnu: " + type)
    }
  )
  rect-box("marker " + label, height, height, stroke-width: 0mm, inner-content: marker)
}

// ----------------------------------------------------------------
// Function: gen-copies
// Description: Génère des copies d'un contenu donné.
// Parameters:
// - exam-id: Identifiant unique de l'examen.
// - copy-content: Contenu à copier.
// - exam-content-hash: Hash du contenu de l'examen.
// - nb-copies: Nombre de copies à générer (valeur par défaut 1).
// - duplex-printing: Impression recto verso (valeur par défaut true).
// - marker-type: Type de marqueur à utiliser (valeur par défaut "qrcode").
// - marker-height: Hauteur du marqueur (valeur par défaut 6mm).
// - marker-content-gutter: Marge entre le marqueur et le contenu (valeur par défaut 2.5mm).
// - page-width: Largeur de la page (valeur par défaut 210mm).
// - page-height: Hauteur de la page (valeur par défaut 297mm).
// - margin-x: Marge horizontale (valeur par défaut 10mm).
// - margin-y: Marge verticale (valeur par défaut 10mm).
// ----------------------------------------------------------------
#let gen-copies(
  exam-id, copy-content, exam-content-hash: "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=",
  nb-copies: 1, duplex-printing: true,
  marker-type: "qrcode",
  marker-height: 6mm, marker-content-gutter: 2.5mm,
  page-width: 210mm, page-height: 297mm,
  margin-x: 10mm, margin-y: 10mm
) = {
  assert.eq(type(exam-id), str, message: "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  assert.eq(type(page-width), length, message: "page-width must be a length")
  assert.eq(type(page-height), length, message: "page-height must be a length")
  assert.eq(type(marker-type), str, message: "marker-type must be a string")
  assert.eq(type(marker-height), length, message: "marker-height must be a length")
  assert.eq(type(margin-x), length, message: "margin-x must be a length")
  assert.eq(type(margin-y), length, message: "margin-y must be a length")
  assert.eq(type(marker-content-gutter), length, message: "marker-content-gutter must be a length")

  let sep = ","  // no longer needed for marker as handled by generate-marker

  set page(
    width: page-width,
    height: page-height,
    margin: (
      x: margin-x,
      y: margin-y + marker-height + marker-content-gutter,
    ),
    header-ascent: 0mm,
    header: {
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
    },
    footer-descent: 0mm,
    footer: {
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
    },
  )

  page-state.update(x => { x.insert("width", page-width.mm()) ; x })
  page-state.update(x => { x.insert("height", page-height.mm()) ; x })

  let pagebreak_to = if duplex-printing { "odd" } else { none }
  let n = 0
  while n < nb-copies {
    copy-counter.update(n)
    n = n + 1
    counter(page).update(1)

    copy-content

    if n < nb-copies {
      pagebreak(to: pagebreak_to)
    }
  }
  finalize-states()
}