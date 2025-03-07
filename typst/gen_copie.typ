#import "style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "common/global_variables.typ": copy-counter, generating-content
#import "common/utils.typ": check-type, update-page-state
#import "common/gen_copy_common.typ": process-copies-with-pagination
#import "components/corner_markers.typ": setup-corner-markers

#let SVG = image("assets/marker.svg")

// Génère des copies d'un contenu donné avec des marqueurs configurables dans les coins.
// - exam-id: Identifiant unique de l'examen.
// - nb-copies: Nombre de copies à générer.
// - duplex-printing: Impression recto verso.
// - marker-config: Configuration des marqueurs pour chaque coin.
// - marker-default-size: Taille par défaut des marqueurs.
// - marker-margin: Marge entre les marqueurs et le bord de la page.
#let gen-copies(
  exam-id,
  nb-copies: 1,
  duplex-printing: true,
  marker-config,
  marker-default-size: 10mm,
  marker-margin: 0mm
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(nb-copies, int, "nb-copies must be an integer")
  check-type(duplex-printing, bool, "duplex-printing must be a boolean")
  check-type(marker-default-size, length, "marker-default-size must be a length")
  check-type(marker-margin, length, "marker-margin must be a length")
  
  update-page-state(PAGE_WIDTH, PAGE_HEIGHT, exam-id)

  let page-fn = () => {
    context if generating-content.get() {
      let positions = (
        top-left: (x: 0pt, y: 0pt),
        top-right: (x: PAGE_WIDTH, y: 0pt),
        bottom-left: (x: 0pt, y: PAGE_HEIGHT),
        bottom-right: (x: PAGE_WIDTH, y: PAGE_HEIGHT)
      )
      
      place(
        dx: positions.top-left.x,
        dy: positions.top-left.y,
        setup-corner-markers(
          top-left-config: marker-config.top-left,
          marker-size: marker-default-size,
          marker-margin: marker-margin
        )
      )
      
      place(
        dx: positions.top-right.x,
        dy: positions.top-right.y,
        setup-corner-markers(
          top-right-config: marker-config.top-right, 
          marker-size: marker-default-size,
          marker-margin: marker-margin
        )
      )
      
      place(
        dx: positions.bottom-left.x,
        dy: positions.bottom-left.y,
        setup-corner-markers(
          bottom-left-config: marker-config.bottom-left,
          marker-size: marker-default-size,
          marker-margin: marker-margin
        )
      )
      
      place(
        dx: positions.bottom-right.x,
        dy: positions.bottom-right.y,
        setup-corner-markers(
          bottom-right-config: marker-config.bottom-right,
          marker-size: marker-default-size,
          marker-margin: marker-margin
        )
      )
      
      place(
        center,
        dy: PAGE_HEIGHT - 15mm,
        block(width: 100%, {
          [Page #counter(page).display() / #context counter(page).final().at(0)]
          linebreak()
          [#exam-id]
        })
      )
    }
  }

  // Configuration de la page - sans marges pour les marqueurs
  set page(
    width: PAGE_WIDTH,
    height: PAGE_HEIGHT,
    margin: 0mm,
    background: page-fn()
  )

  // Applique les marges uniquement au contenu
  let content-with-margins(content) = {
    pad(x: MARGIN_X, y: MARGIN_Y, content)
  }

  // Traitement des copies avec le contenu dans les marges
  let original-process = process-copies-with-pagination
  let custom-process(nb-copies, duplex-printing) = {
    original-process(nb-copies, duplex-printing, content-wrapper: content-with-margins)
  }

  custom-process(nb-copies, duplex-printing)
}

#let marker-config = (
  top-left: (
    type: "svg",
    svg: SVG,
  ),
  top-right: (
    type: "svg",
    svg: SVG,
  ),
  bottom-left: (
    type: "aztec",
    size: 10mm,
  ),
  bottom-right: (
    type: "svg",
    svg: SVG,
  )
)

#let m-s = float(sys.inputs.at("marker-default-size", default: "3")) * 1mm
#let m-m = float(sys.inputs.at("marker-margin", default: "3")) * 1mm
#let n-c = int(sys.inputs.at("nb-copies", default: "1"))
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1

#gen-copies(
  "Le pire examen de tous les temps !", 
  nb-copies: n-c, 
  duplex-printing: d-p,
  marker-config,
  marker-default-size: m-s,
  marker-margin: m-m
)