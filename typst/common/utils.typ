// ----------------------------------------------------------------
// Permet de vérifier le type d'une variable
// Parameters:
// - param: La variable à vérifier.
// - expected: Le type attendu.
// - msg: Message d'erreur à afficher si le type ne correspond pas.
// ----------------------------------------------------------------
#let check-type = (param, expected, msg) => {
  assert.eq(type(param), expected, message: msg)
}