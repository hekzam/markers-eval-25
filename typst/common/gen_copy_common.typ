#import "../common/global_variables.typ": copy-counter
#import "../content/content.typ": content
#import "../common/utils.typ": update-page-state, finalize-states
#import "../style/const.typ": PAGE_WIDTH, PAGE_HEIGHT

// ----------------------------------------------------------------
// Génère des copies d'un contenu donné.
// - nb-copies: Nombre de copies à générer.
// - duplex-printing: Impression recto verso.
// ----------------------------------------------------------------
#let process-copies-with-pagination = (nb-copies, duplex-printing) => {
  update-page-state(PAGE_WIDTH, PAGE_HEIGHT)
  
  let pagebreak_to = if duplex-printing { "odd" } else { none }
  let copy-index = 0
  while copy-index < nb-copies {
    copy-counter.update(copy-index)
    copy-index = copy-index + 1
    counter(page).update(1)

    content // Contenu de la copie

    if copy-index < nb-copies {
      pagebreak(to: pagebreak_to)
    }
  }
  finalize-states()
}