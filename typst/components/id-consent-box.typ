#import "@preview/suiji:0.3.0": *
#import "./container.typ": container

#let exo-pts(pts) = {
  h(1fr)
  text(size: 10pt, weight: "regular")[(#pts points)]
}

#let id-consent-box(seed,
  digitbox-width: 6mm,
  digitbox-height: 8mm,
  fillbox-height: 8mm,
  checkbox-width: 3mm,
  checkbox-height: 3mm,
) = {
  let rng = gen-rng-f(seed)
  let (rng, shuffled-digits) = shuffle-f(rng, range(10))

  rect(width: 100%, height: auto, fill: white,
        stroke: (paint: black, thickness:.2mm, dash: "dotted", cap: "square", join: "bevel"))[
    #grid(columns: 2, column-gutter: 5mm,
      [Student number #box(grid(columns: 8, column-gutter: 0mm,
        ..range(8).map(x => container(("id", "number", str(x)).join("-"), digitbox-width, digitbox-height)),
      ))],
      [Student name #box(container("id-name", 8cm, fillbox-height))]
    )

    To improve automatic recognition of your entries, perform the requested operation on each box.
    #v(-.5em)
    #grid(columns: 4, column-gutter: (5mm, 5mm, 14mm),
      [#box(container("calib-shaded", checkbox-width, checkbox-height)) Blacken],
      [#box(container("calib-empty", checkbox-width, checkbox-height)) Leave blank],
      [#box(container("calib-ticked", checkbox-width, checkbox-height)) Tick],
      grid(
        columns: 2,
        column-gutter: 2mm,
        [#v(0.5mm) Write each digit],
        grid(
          columns: 10,
          column-gutter: 0mm,
          row-gutter: .5em,
          align: center,
          ..shuffled-digits.map(digit => [
            #v(0.5mm)
            #digit
          ]),
          ..shuffled-digits.map(digit => [
            #container(("calib", "digit", str(digit)).join("-"), digitbox-width, digitbox-height)
          ]),
        )
      )
    )

    I consent to the use of my data (blacked-out and checked boxes, handwritten numbers) for research purposes in order to evaluate and develop tools for the automatic correction of examination papers and character recognition.
    #v(-.5em)
    #grid(
      columns: 2, column-gutter: 5mm,
      [#box(container("consent-use-yes", checkbox-width, checkbox-height)) Yes],
      [#box(container("consent-use-no", checkbox-width, checkbox-height)) No],
    )

    I consent to the anonymized publication of my entries (blacked-out and checked boxes, handwritten numbers) in the form of a database under the Creative Commons Attribution-ShareAlike 4.0 International free license.
    #v(-.5em)
    #grid(
      columns: 2, column-gutter: 5mm,
      [#box(container("consent-opendata-yes", checkbox-width, checkbox-height)) Yes],
      [#box(container("consent-opendata-no", checkbox-width, checkbox-height)) No],
    )

    Please contact Millian Poquet (`millian.poquet@irit.fr`) if you wish to withdraw your consent.
  ]
}
