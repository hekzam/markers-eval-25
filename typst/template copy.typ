#import "src/gen_copy.typ": gen-copies
#import "style/const.typ": init-style-params

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

// Initialisation des paramètres de style
#init-style-params()

// Paramètres de génération
#let n-c = int(sys.inputs.at("nb-copies", default: "1")) // Ne fonctionne pas a plus de 1 copie
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1
#let marker_types = sys.inputs.at("marker-types", default: "(qrcode,qrcode,qrcode,qrcode,rmqr)")

#let marker_config = parse-marker-types(marker_types)

#gen-copies(
  "Le pire examen de tous les temps !", 
  nb-copies: n-c, 
  duplex-printing: d-p,
  marker_config
)