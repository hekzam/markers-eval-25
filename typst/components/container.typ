#import "../common/global_variables.typ": atomic-boxes, copy-counter

/**
 * Crée un conteneur pour n'importe quel contenu.
 * @param id Identifiant du conteneur
 * @param width Largeur du conteneur
 * @param height Hauteur du conteneur
 * @param fill-color Couleur de remplissage (valeur par défaut: blanc)
 * @param stroke-width Largeur de la bordure (valeur par défaut: 0.25mm)
 * @param stroke-color Couleur de la bordure (valeur par défaut: noir)
 * @param inner-content Contenu à l'intérieur du conteneur
 */
#let container(id, width, height, fill-color: white, stroke-width: 0.25mm, stroke-color: black, inner-content: []) = {
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-width), length, message: "stroke must be a length")

  rect(width: width, height: height, fill: fill-color, stroke: stroke-width + stroke-color, radius: 0mm, inset: 0mm, outset: 0mm, {
    context if copy-counter.get().at(0) == 0 {
      layout(size => {
        let pos = here().position()
        let box = (
          page: pos.page,
          x: pos.x.mm(),
          y: pos.y.mm(),
          width: if type(width) == length { width.mm() }
            else if type(width) == ratio { (width * size.width).mm() },
          height: if type(height) == length { height.mm() }
              else if type(height) == ratio { (height * size.height).mm() },
          stroke-width: stroke-width.mm(),
          stroke-color: stroke-color.to-hex(),
          fill-color: fill-color.to-hex(),
        )
        atomic-boxes.update(x => { x.insert(str(id), box) ; x })
        inner-content
      })
    } else {
      inner-content
    }
  })
}