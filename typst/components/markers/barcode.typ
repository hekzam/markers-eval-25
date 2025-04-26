#import "@preview/tiaoma:0.2.1"
#import "../container.typ": gen-box

#let HEADER_MARKER_TYPE = ("rmqr", "code128", "pdf417")
#let CORNER_MARKER_TYPE = ("qrcode", "microqr", "aztec", "datamatrix")

/**
 * Génère un barcode à partir des données fournies et l'encapsule dans une boîte.
 * @param label Un identifiant ou une étiquette pour la boîte
 * @param parts Une liste de chaînes représentant les données stockées dans le code-barre
 * @param height La hauteur du code-barre
 * @param type Type de barcode (valeur par défaut "qrcode")
 * @param color Couleur du code-barre
 */
#let barcode(label, parts, height, type: "qrcode", color) = {
  let sep = ","
  let data = parts.join(sep)

  // Netoyyer le type : Enlever le suffixe "-encoded" si présent
  let type = type.split("-encoded").at(0)


  let barcode_fns = (
    "qrcode": tiaoma.qrcode,
    "microqr": tiaoma.micro-qr,
    "aztec": tiaoma.aztec,
    "datamatrix": tiaoma.data-matrix,
    "pdf417": tiaoma.pdf417,
    "rmqr": tiaoma.rmqr,
    "code128": tiaoma.code128,
    default: tiaoma.qrcode,
  )

  let box_width = if HEADER_MARKER_TYPE.contains(type) { 120mm } else { height }
  let box_height = if HEADER_MARKER_TYPE.contains(type) { 10mm } else { height }
  let options = (fg-color: color)

  let barcode = barcode_fns.at(type)(data, height: height, options: options)

  gen-box(label, box_width, box_height, stroke-width: 0mm, inner-content: barcode)
}
