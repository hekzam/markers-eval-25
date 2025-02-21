#import "@preview/tiaoma:0.2.0"

#let atomic-boxes = state("atomic-boxes", (:))
#let page-state = state("page-state", (:))
#let copy-counter = counter("copy-counter")
#let generating-content = state("generating-content", true)

#let finalize-states() = context [
  #metadata(atomic-boxes.final()) <atomic-boxes>
  #metadata(page-state.final()) <page>
]

#let circle-box(id, diameter, fill-color: white, stroke-width: 0.25mm, stroke-color: black, inner-content: []) = {
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-width), length, message: "stroke must be a length")

  set align(left+top)
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


#let rect-box(id, width, height, fill-color: white, stroke-width: 0.25mm, stroke-color: black, inner-content: []) = {
  assert.eq(type(fill-color), color, message: "fill-color must be a color")
  assert.eq(rgb(fill-color).components().at(3), 100%, message: "fill-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-color), color, message: "stroke-color must be a color")
  assert.eq(rgb(stroke-color).components().at(3), 100%, message: "stroke-color must be fully opaque (no alpha or alpha=100%)")
  assert.eq(type(stroke-width), length, message: "stroke must be a length")

  set align(left+top)
  rect(width: width, height: height, fill: fill-color, stroke: stroke-width + stroke-color, radius: 0mm, inset: 0mm, outset: 0mm, {
    context if copy-counter.get().at(0) == 0 {
      layout(size => {
        set align(left+top)
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

#let gen-copies(
    exam-id, copy-content, exam-content-hash: "qhj6DlP5gJ+1A2nFXk8IOq+/TvXtHjlldVhwtM/NIP4=",
    nb-copies: 1, duplex-printing: true,
    page-width: 210mm, page-height: 297mm, barcode-height: 6mm,
    margin-x: 10mm, margin-y: 10mm, barcode-content-gutter: 2.5mm,
) = {
    assert.eq(type(exam-id), str, message: "exam-id must be a string")
    assert(not exam-id.contains(","), message: "exam-id cannot contain comma ','")
    assert.eq(type(page-width), length, message: "page-width must be a length")
    assert.eq(type(page-height), length, message: "page-height must be a length")
    assert.eq(type(barcode-height), length, message: "barcode-height must be a length")
    assert.eq(type(margin-x), length, message: "margin-x must be a length")
    assert.eq(type(margin-y), length, message: "margin-y must be a length")
    assert.eq(type(barcode-content-gutter), length, message: "barcode-content-gutter must be a length")

    let sep = ","

    set page(
      width: page-width,
      height: page-height,
      margin: (
        x: margin-x,
        y: margin-y + barcode-height + barcode-content-gutter,
      ),
      header-ascent: 0mm,
      header: {
        set align(top)
        context if generating-content.get() {
          v(margin-y)
          grid(
            columns: 2,
            column-gutter: 1fr,
            {
              let page-i = counter(page).get().at(0)
              let diameter = barcode-height // Remplace la taille du QR code par un diamètre de cercle
              circle-box("marker circle tl page" + str(page-i), diameter, fill-color: black, stroke-width: 0mm, inner-content: [])
            },
            {
              let page-i = counter(page).get().at(0)
              let diameter = barcode-height
              circle-box("marker circle tr page" + str(page-i), diameter, fill-color: black, stroke-width: 0mm, inner-content: [])
            },
          )
        } else []
      },
      footer-descent: 0mm,
      footer: {
        set align(bottom)
        context if generating-content.get() {
          grid(
            columns: 3,
            column-gutter: 1fr,
            {
              let copy-i = copy-counter.get().at(0)
              let page-i = counter(page).get().at(0)
              let diameter = barcode-height
              circle-box("marker circle bl page" + str(page-i), diameter, fill-color: black, stroke-width: 0mm, inner-content: [])
            },
            {
              grid(
                columns: 1,
                row-gutter: 2mm,
                align: center,
                [
                  #exam-id #copy-counter.display()
                ],
                [
                  page #counter(page).display() / #context counter(page).final().at(0)
                ],
              )
            },
            {
              let copy-i = copy-counter.get().at(0)
              let page-i = counter(page).get().at(0)
              let diameter = barcode-height
              circle-box("marker circle br page" + str(page-i), diameter, fill-color: black, stroke-width: 0mm, inner-content: [])
            },
          )
          v(margin-y)
        } else []
      },
    )

    page-state.update(x => { x.insert("width", page-width.mm()) ; x })
    page-state.update(x => { x.insert("height", page-height.mm()) ; x })

    let pagebreak_to = if duplex-printing { "odd" } else { none }
    let n = 0
    while n < nb-copies {
      copy-counter.update(n)
      n = n + 1
      counter(page).update(1)

      copy-content

      if n < nb-copies {
        pagebreak(to: pagebreak_to)
      }
    }
    finalize-states()
  }
