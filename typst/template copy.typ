#import "src/gen_copy.typ": gen-copies

/**
 * Parse une string de types de marqueurs et retourne une configuration complète.
 * Format attendu : "(type_tl,type_tr,type_bl,type_br,type_header)"
 */
#let parse-marker-types(types-str) = {
  let clean = types-str.trim().slice(1, -1)
  let parts = clean.split(",").map((x)=>{x.trim()})
  (
    top-left: parts.at(0, default: "none"),
    top-right: parts.at(1, default: "none"),
    bottom-left: parts.at(2, default: "none"),
    bottom-right: parts.at(3, default: "none"),
    header: parts.at(4, default: "none")
  )
}

// Paramètres pour la génération de copies
#let e-m-s = float(sys.inputs.at("encoded-marker-size", default: "15")) * 1mm
#let f-m-s = float(sys.inputs.at("fiducial-marker-size", default: "3")) * 1mm
#let h-m-s = float(sys.inputs.at("header-marker-size", default: "7")) * 1mm
#let s-w = float(sys.inputs.at("stroke-width", default: "2")) * 1mm
#let m-m = float(sys.inputs.at("marker-margin", default: "3")) * 1mm
#let n-c = int(sys.inputs.at("nb-copies", default: "1")) // Ne fonctionne pas a plus de 1 copie
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1
#let marker_types = sys.inputs.at("marker-types", default: "(qrcode,qrcode,qrcode,qrcode,rmqr)")
#let g-l = int(sys.inputs.at("grey-level", default: "0"))

#let marker_config = parse-marker-types(marker_types)

#gen-copies(
  "Le pire examen de tous les temps !", 
  nb-copies: n-c, 
  duplex-printing: d-p,
  marker_config,
  e-m-s,
  f-m-s,
  // e-m-s, // TODO: remove this line when the bug is fixed
  s-w,
  m-m,
  g-l
)