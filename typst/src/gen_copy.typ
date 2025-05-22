#import "@preview/suiji:0.3.0": *
#import "../common/global_variables.typ": copy-counter, generating-content
#import "../common/utils.typ": check-type, update-page-state, finalize-states
#import "../components/corner_markers.typ": setup-corner-markers, place-header-marker
#import "../content/content.typ": content
#import "../content/presentation-grid.typ": presentation-grid-1, presentation-grid-2
#import "../content/understanding.typ": understanding-1, understanding-2

#let PAGE_WIDTH = 210mm
#let PAGE_HEIGHT = 297mm

/**
 * Génère un contenu aléatoire à partir d'une liste de blocs de contenu.
 * @param seed Graine pour le générateur aléatoire
 * @return Un bloc de contenu aléatoire
 */
#let get-random-content(seed: 50) = {
  let rng = gen-rng-f(seed)

  let blocks = (
    content,
    understanding-1,
    understanding-2,
    presentation-grid-1,
    presentation-grid-2,
  )

  let (rng,arr) = integers-f(rng, low: 0, high: blocks.len(), size: 1)

  blocks.at(arr.at(0))
}

/**
 * Génère un nombre donné de copies d'un contenu, en gérant la pagination
 * @param nb-copies Nombre de copies à générer
 * @param duplex-printing Impression recto verso
 * @param content-wrapper Fonction qui génère le contenu de chaque copie
 * @param seed Graine pour la génération aléatoire du contenu
 */
#let process-copies-with-pagination(nb-copies, duplex-printing, seed: 42, content-wrapper: (c) => c) = {
  
  copy-counter.update(0)
  
  content-wrapper(get-random-content(seed: seed))
  
  for i in range(1, nb-copies) {
    if duplex-printing and calc.odd(i) {
      pagebreak(to: "odd")
    } else {
      pagebreak()
    }
    copy-counter.update(i)
    content-wrapper(get-random-content(seed: seed + i))
  }

  finalize-states()
}

/**
 * Génère des copies d'un contenu donné avec des marqueurs configurables dans les coins.
 * @param exam-id Identifiant unique de l'examen
 * @param nb-copies Nombre de copies à générer
 * @param duplex-printing Impression recto verso
 * @param marker-config Configuration des marqueurs pour chaque coin et en-tête
 * @param style-params Paramètres de style pour les marqueurs
 * @param should-generate-content Indique si le contenu et le repère de pagination doivent être générés
 * @param seed Graine pour la génération aléatoire du contenu
 */
#let gen-copies(
  exam-id,
  nb-copies,
  duplex-printing,
  marker-config,
  style-params,
  should-generate-content,
  seed: 42
) = {
  check-type(exam-id, str, "exam-id must be a string")
  assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
  check-type(nb-copies, int, "nb-copies must be an integer")
  check-type(duplex-printing, bool, "duplex-printing must be a boolean")
  check-type(should-generate-content, bool, "should-generate-content must be a boolean")

  update-page-state(PAGE_WIDTH, PAGE_HEIGHT, exam-id)

  let page-fn = () => {
    context {
      let positions = (
        top-left:    (x: 0pt,         y: 0pt),
        top-right:   (x: PAGE_WIDTH,  y: 0pt),
        bottom-left: (x: 0pt,       y: PAGE_HEIGHT),
        bottom-right:(x: PAGE_WIDTH,  y: PAGE_HEIGHT)
      )

      let place-corner = (corner) => {
        let pos = positions.at(corner)
        
        if marker-config.at(corner) != "#" {
          place(
            dx: pos.x,
            dy: pos.y,
            setup-corner-markers(
              corner,
              marker-config.at(corner),
              style-params
            )
          )
        }
      }

      // Place les marqueurs dans les coins de la page
      for corner in ("top-left", "top-right", "bottom-left", "bottom-right") {
        place-corner(corner)
      }

      // Place le marqueur d'en-tête au centre haut de la page
      if marker-config.at("header") != "#" {
        place-header-marker(marker-config.at("header"), style-params)
      }
      
      // Place un repère de pagination en bas au centre de la page
      if should-generate-content {
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
    let margin-y = style-params.content_margin_y+style-params.marker_margin+calc.max(style-params.encoded_marker_size, style-params.unencoded_marker_size)
    context {
      if should-generate-content {
        pad(x: style-params.content_margin_x, y: margin-y, content)
      } else {
        none
      }
    }
  }

  // Traitement des copies avec le contenu dans les marges
  let original-process = process-copies-with-pagination
  let custom-process(nb-copies, duplex-printing) = {
    original-process(nb-copies, duplex-printing, seed: seed, content-wrapper: content-with-margins)
  }

  custom-process(nb-copies, duplex-printing)
}