#import "../../common/global_variables.typ": atomic-boxes, copy-counter

// Crée une boîte circulaire avec un identifiant unique et des propriétés spécifiques.
// - id: Identifiant unique de la boîte.
// - diameter: Diamètre de la boîte.
// - fill-color: Couleur de remplissage.
// - stroke-width: Épaisseur du trait.
// - stroke-color: Couleur du trait.
// - inner-content: Contenu à afficher à l'intérieur de la boîte.
#let circle-box(id, diameter, fill-color: black, stroke-width: 0.25mm, stroke-color: black, inner-content: []) = {
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-width), length, message: "stroke must be a length")

  circle(radius: diameter / 2, fill: fill-color, stroke: stroke-width + stroke-color, {
    context if copy-counter.get().at(0) == 0 {
      layout(size => {
        set align(left+top)
        let pos = here().position()
        let box = (
          page: pos.page,
          x: pos.x.mm(),
          y: pos.y.mm(),
          diameter: diameter.mm(),
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