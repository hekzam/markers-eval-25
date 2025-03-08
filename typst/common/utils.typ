#import "./global_variables.typ": atomic-boxes, copy-counter, page-state

/**
 * Permet de vérifier le type d'une variable
 * @param param La variable à vérifier
 * @param expected Le type attendu
 * @param msg Message d'erreur à afficher si le type ne correspond pas
 */
#let check-type = (param, expected, msg) => {
  assert.eq(type(param), expected, message: msg)
}


/**
 * Permet de récupérer les index de la copie et de la page courante
 * @return Tuple contenant l'index de la copie et l'index de la page
 */
#let get-indexes = () => {
  let copy-i = copy-counter.get().at(0)
  let page-i = counter(page).get().at(0)
  (copy-i, page-i)
}

/**
 * Récupère l'identifiant unique de l'examen
 * @return L'identifiant de l'examen
 */
#let get-exam-id = () => {
  page-state.get().at("exam-id")
}

/**
 * Finalise les états de la page et des boîtes atomiques.
 */
#let finalize-states() = context [
  #metadata(atomic-boxes.final()) <atomic-boxes>
  #metadata(page-state.final()) <page>
]

/**
 * Met à jour l'état de la page avec les dimensions fournies.
 * @param width Largeur de la page
 * @param height Hauteur de la page
 * @param exam-id Identifiant unique de l'examen
 */
#let update-page-state = (width, height, exam-id) => {
  page-state.update(x => {
    x.insert("width", width.mm())
    x.insert("height", height.mm())
    x.insert("exam-id", exam-id)
    x
  })
}