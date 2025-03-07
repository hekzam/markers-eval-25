
#import "src/gen_copy.typ": gen-copies
#let SVG = image("assets/marker.svg")

#let marker-config = (
  top-left: (
    type: "svg",
    svg: SVG,
  ),
  top-right: (
    type: "svg",
    svg: SVG,
  ),
  bottom-left: (
    type: "aztec",
    size: 10mm,
  ),
  bottom-right: (
    type: "svg",
    svg: SVG,
  )
)

#let m-s = float(sys.inputs.at("marker-default-size", default: "3")) * 1mm
#let m-m = float(sys.inputs.at("marker-margin", default: "3")) * 1mm
#let n-c = int(sys.inputs.at("nb-copies", default: "1"))
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1

#gen-copies(
  "Le pire examen de tous les temps !", 
  nb-copies: n-c, 
  duplex-printing: d-p,
  marker-config,
  marker-default-size: m-s,
  marker-margin: m-m
)