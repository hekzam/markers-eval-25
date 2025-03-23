#import "@preview/tiaoma:0.2.1"
#import "../container.typ": gen-box

/**
 * Génère un barcode à partir des données fournies et l'encapsule dans une boîte.
 * @param label Un identifiant ou une étiquette pour la boîte
 * @param parts Une liste de chaînes représentant les données stockées dans le code-barre
 * @param width La largeur de la boîte
 * @param height La hauteur de la boîte
 * @param type Type de barcode (valeur par défaut "qrcode")
 * @param color Couleur du code-barre
 */
#let barcode(label, parts, width, height, type: "qrcode", color) = {
  let sep = ","
  let data = parts.join(sep)
  if (parts.len() == 0) {
    data = "90383"
  }
  let options = (
    fg-color: color
  )
  let barcode = (
    if type == "qrcode" {
      tiaoma.qrcode(data, height: height, options: options)
    } else if type == "aztec" {
      tiaoma.micro-qr(data, height: height, options: options)
    } else if type == "datamatrix" {
      tiaoma.data-matrix(data, height: height, options: options)
    } else if type == "pdf417-comp" {
      tiaoma.pdf417-comp(data, height: height, options: options)
    } else {
      error("Type de barcode inconnu: " + type)
    }
  )
  gen-box(label, height, height, stroke-width: 0mm, inner-content: barcode)
}
