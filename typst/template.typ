
#import "src/gen_copy.typ": gen-copies
#let AR_1 = image("assets/4x4_1000-190.svg")
#let AR_2 = image("assets/4x4_1000-997.svg")
#let AR_3 = image("assets/4x4_1000-999.svg")

#let marker-config = (
  top-left: (
    type: "svg",
    svg: AR_1,
  ),
  top-right: (
    type: "svg",
    svg: AR_2,
  ),
  bottom-left: (
    type: "svg",
    svg: AR_3,
  ),
  bottom-right: (
    type: "qrcode",
    size: 17mm
  )
)

#let m-s = float(sys.inputs.at("marker-default-size", default: "4")) * 1mm
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