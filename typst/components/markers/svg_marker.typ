#import "../container.typ": gen-box

/**
 * Génère une boîte contenant une image SVG pour les marqueurs de coin.
 * @param id Identifiant unique de la boîte
 * @param svg Image SVG à afficher dans la boîte
 * @param height Hauteur de la boîte
 */
#let gen-svg-box(
  id,
  svg,
  height
) = {
  set align(bottom)
  gen-box(
    id,
    height,
    height,
    stroke-width: 0mm,
    inner-content: svg
  )
}
