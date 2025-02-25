#import "@preview/tiaoma:0.2.1"
#import "../container.typ": gen-box

// ----------------------------------------------------------------
// Génère un marker à partir des données fournies et l'encapsule dans une boîte.
// - label: Un identifiant ou une étiquette pour la boîte.
// - parts: Une liste de chaînes représentant les données stockées dans le code-barre.
// - height: La hauteur (et largeur) de la boîte.
// - type: Type de marqueur (valeur par défaut "qrcode").
// ----------------------------------------------------------------
#let generate-marker(label, parts, height, type: "qrcode") = {
  let sep = ","
  let data = parts.join(sep)
  let marker = (
    if type == "qrcode" {
      tiaoma.qrcode(data, height: height)
    } else if type == "aztec" {
      tiaoma.aztec(data, height: height)
    } else if type == "datamatrix" {
      tiaoma.data-matrix(data, height: height)
    } else if type == "pdf417-comp" {
      tiaoma.pdf417-comp(data, height: height)
    } else {
      error("Type de marqueur inconnu: " + type)
    }
  )
  gen-box("marker barcode " + label, height, height, stroke-width: 0mm, inner-content: marker)
}
