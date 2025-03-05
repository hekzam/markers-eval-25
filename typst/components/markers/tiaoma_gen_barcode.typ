#import "@preview/tiaoma:0.2.1"
#import "../container.typ": gen-box

// ----------------------------------------------------------------
// Génère un barcode à partir des données fournies et l'encapsule dans une boîte.
// - label: Un identifiant ou une étiquette pour la boîte.
// - parts: Une liste de chaînes représentant les données stockées dans le code-barre.
// - height: La hauteur (et largeur) de la boîte.
// - type: Type de barcode (valeur par défaut "qrcode").
// ----------------------------------------------------------------
#let barcode(label, parts, width, height, type: "qrcode") = {
  let sep = ","
  let data = parts.join(sep)
  if (parts.len() == 0) {
    data = "90383"
  }
  let barcode = (
    if type == "qrcode" {
      tiaoma.qrcode(data, height: height)
    } else if type == "aztec" {
      tiaoma.micro-qr(data, height: height)
    } else if type == "datamatrix" {
      tiaoma.data-matrix(data, height: height)
    } else if type == "pdf417-comp" {
      tiaoma.pdf417-comp(data, height: height)
    } else {
      error("Type de barcode inconnu: " + type)
    }
  )
  gen-box(label, height, height, stroke-width: 0mm, inner-content: barcode)
}
