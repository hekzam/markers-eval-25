#import "./global_variables.typ": atomic-boxes, copy-counter, page-state

// ----------------------------------------------------------------
// Permet de vérifier le type d'une variable
// - param: La variable à vérifier.
// - expected: Le type attendu.
// - msg: Message d'erreur à afficher si le type ne correspond pas.
// ----------------------------------------------------------------
#let check-type = (param, expected, msg) => {
  assert.eq(type(param), expected, message: msg)
}


// Permet de récupérer les index de la copie et de la page courante
#let get-indexes = () => {
  let copy-i = copy-counter.get().at(0)
  let page-i = counter(page).get().at(0)
  (copy-i, page-i)
}

// Finalise les états de la page et des boîtes atomiques.
#let finalize-states() = context [
  #metadata(atomic-boxes.final()) <atomic-boxes>
  #metadata(page-state.final()) <page>
]

// ----------------------------------------------------------------
// Met à jour l'état de la page avec les dimensions fournies.
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