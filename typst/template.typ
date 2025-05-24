#import "src/gen_copy.typ": gen-copies

/**
 * Parse une string de types de marqueurs et retourne une configuration complète.
 * Format attendu : "(type_tl,type_tr,type_bl,type_br,type_header)"
 */
#let parse-marker-types(types-str) = {
  let clean = types-str.trim().slice(1, -1)
  let parts = clean.split(",").map(x => { x.trim() })
  (
    top-left: parts.at(0, default: "#"),
    top-right: parts.at(1, default: "#"),
    bottom-left: parts.at(2, default: "#"),
    bottom-right: parts.at(3, default: "#"),
    header: parts.at(4, default: "#"),
  )
}

#let style-params = (
  encoded_marker_size: float(sys.inputs.at("encoded-marker-size", default: "15")) * 1mm,
  unencoded_marker_size: float(sys.inputs.at("unencoded-marker-size", default: "3")) * 1mm,
  header_marker_size: float(sys.inputs.at("header-marker-size", default: "7")) * 1mm,
  stroke_width: float(sys.inputs.at("stroke-width", default: "2")) * 1mm,
  marker_margin: float(sys.inputs.at("marker-margin", default: "3")) * 1mm,
  grey_level: int(sys.inputs.at("grey-level", default: "0")),
  content_margin_x: float(sys.inputs.at("content-margin-x", default: "10")) * 1mm,
  content_margin_y: float(sys.inputs.at("content-margin-y", default: "10")) * 1mm,
)

// Paramètres de génération
#let nb-copies = int(sys.inputs.at("nb-copies", default: "1")) // Ne fonctionne pas a plus de 1 copie
#let duplex-printing = int(sys.inputs.at("duplex-printing", default: "0")) == 1
#let marker-types = sys.inputs.at("marker-types", default: "(qrcode,qrcode,qrcode,qrcode,rmqr)")
#let should-generate-content = int(sys.inputs.at("generating-content", default: "1")) == 1
#let seed = int(sys.inputs.at("seed", default: "42"))

#let marker-config = parse-marker-types(marker-types)

#gen-copies(
  "Le pire examen de tous les temps !",
  nb-copies,
  duplex-printing,
  marker-config,
  style-params,
  should-generate-content,
  seed:seed,
)
