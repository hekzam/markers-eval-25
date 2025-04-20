#import "../../common/global_variables.typ": atomic-boxes, copy-counter

// Crée une boîte triangulaire avec un identifiant unique et des propriétés spécifiques.
// - id: Identifiant unique de la boîte.
// - side: Longueur du côté du triangle équilatéral.
// - fill-color: Couleur de remplissage.
// - stroke-width: Épaisseur du trait.
// - stroke-color: Couleur du trait.
#let triangle-box(id, side, fill-color: black, stroke-width: 0.5mm, stroke-color: black) = {
  assert.eq(type(side), length, message: "side must be a length")
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque")
  assert.eq(type(stroke-width), length, message: "stroke-width must be a length")

  polygon.regular(
    vertices: 3,
    size: side,
    fill: fill-color
  )

  context if copy-counter.get().at(0) == 0 {
    layout(size => {
      set align(left + bottom)
      let pos = here().position()
      let box = (
        page: pos.page,
        x: pos.x.mm(),
        y: pos.y.mm(),
        width: size.width.mm(),
        height: size.height.mm(),
        stroke-width: stroke-width.mm(),
        stroke-color: stroke-color.to-hex(),
        fill-color: fill-color.to-hex(),
      )
      atomic-boxes.update(x => { x.insert(str(id), box); x })
    })
  }
}
