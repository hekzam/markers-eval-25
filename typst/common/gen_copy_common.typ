#import "global_variables.typ": copy-counter, generating-content
#import "../content/content.typ": content
#import "../common/utils.typ": update-page-state, finalize-states


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