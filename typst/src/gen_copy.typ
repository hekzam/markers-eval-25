#import "../style/const.typ": PAGE_WIDTH, PAGE_HEIGHT, MARGIN_X, MARGIN_Y
#import "../common/global_variables.typ": copy-counter, generating-content
#import "../common/utils.typ": check-type, update-page-state, finalize-states
#import "../components/corner_markers.typ": setup-corner-markers
#import "../content/content.typ": content

/**
 * Génère un nombre donné de copies d'un contenu, en gérant la pagination
 * @param nb-copies Nombre de copies à générer
 * @param duplex-printing Impression recto verso
 * @param content-wrapper Fonction qui génère le contenu de chaque copie
 */
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

/**
 * Génère des copies d'un contenu donné avec des marqueurs configurables dans les coins.
 * @param exam-id Identifiant unique de l'examen
 * @param nb-copies Nombre de copies à générer
 * @param duplex-printing Impression recto verso
 * @param marker-config Configuration des marqueurs pour chaque coin
 * @param encoded-marker-size Taille du marqueur encodé
 * @param fiducial-marker-size Taille du marqueur fiducial
 * @param marker-margin Marge entre les marqueurs et le bord de la page
 */
#let gen-copies(
  exam-id,
  nb-copies: 1,
  duplex-printing: true,
  marker-config,
  encoded-marker-size,
  fiducial-marker-size,
  marker-margin
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(nb-copies, int, "nb-copies must be an integer")
  check-type(duplex-printing, bool, "duplex-printing must be a boolean")
  check-type(encoded-marker-size, length, "encoded-marker-size must be a length")
  check-type(fiducial-marker-size, length, "fiducial-marker-size must be a length")
  check-type(marker-margin, length, "marker-margin must be a length")
  
  update-page-state(PAGE_WIDTH, PAGE_HEIGHT, exam-id)

  let page-fn = () => {
    context if generating-content.get() {
      let positions = (
        top-left:    (x: 0pt,         y: 0pt),
        top-right:   (x: PAGE_WIDTH,  y: 0pt),
        bottom-left: (x: 0pt,       y: PAGE_HEIGHT),
        bottom-right:(x: PAGE_WIDTH,  y: PAGE_HEIGHT)
      )

      let place-corner = (corner) => {
        let pos = positions.at(corner)
        let config-key = corner + "-config"
        
        place(
          dx: pos.x,
          dy: pos.y,
          setup-corner-markers(
            config-key,                // Argument positionnel 1: clé de configuration
            marker-config.at(corner),  // Argument positionnel 2: configuration du coin
            encoded-marker-size,       // Argument positionnel 3: taille du marqueur encodé
            fiducial-marker-size,      // Argument positionnel 4: taille du marqueur fiducial 
            marker-margin              // Argument positionnel 5: marge
          )
        )
      }

      for corner in ("top-left", "top-right", "bottom-left", "bottom-right") {
        place-corner(corner)
      }
      
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