#import "../container.typ": gen-box
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
