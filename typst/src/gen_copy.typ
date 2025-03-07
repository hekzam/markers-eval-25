#import "../style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "../common/global_variables.typ": copy-counter, generating-content
#import "../common/utils.typ": check-type, update-page-state, finalize-states
#import "../components/corner_markers.typ": setup-corner-markers
#import "../content/content.typ": content

// Génère un nombre donné de copies d'un contenu, en gérant la pagination
// - nb-copies: Nombre de copies à générer
// - duplex-printing: Impression recto verso
// - content-wrapper: Fonction qui génère le contenu de chaque copie
#let process-copies-with-pagination(nb-copies, duplex-printing, content-wrapper: (c) => c) = {
  
  // Initialisation des états
  copy-counter.update(1)
  generating-content.update(true)
  
  // Affichage de la première copie
  content-wrapper(content)
  
  // Génération des copies suivantes
  for i in range(2, nb-copies + 1) {
    if duplex-printing and calc.odd(i) {
      pagebreak(to: "odd")
    } else {
      pagebreak()
    }
    copy-counter.update(i)
    content-wrapper(content)
  }

  generating-content.update(false)
  finalize-states()
}

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