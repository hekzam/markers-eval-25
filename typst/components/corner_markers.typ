#import "../common/global_variables.typ": generating-content
#import "../common/utils.typ": get-indexes, get-exam-id
#import "./markers/tiaoma_gen_barcode.typ": barcode
#import "./markers/circle.typ": circle-box
#import "./markers/svg_marker.typ": gen-svg-box

// Crée un marqueur à partir d'une configuration donnée
// - position: Position du marqueur
// - marker-config: Configuration du marqueur
// - size: Taille par défaut du marqueur
#let create-marker(position, marker-config, size) = {
  if marker-config == none { return none }
  
  let type = marker-config.at("type", default: "qrcode")
  let marker-size = marker-config.at("size", default: size)
  
  let prefix = position
  
  if type == "qrcode" or type == "datamatrix" or type == "aztec" or type == "pdf417-comp" {
    let (copy-i, page-i) = get-indexes()
    let content-hash = marker-config.at("content-hash", default: "")
    
    let barcode-data = (prefix, str(copy-i), str(page-i), content-hash, get-exam-id())
    
    return barcode(prefix, barcode-data, marker-size, marker-size, type: type)
  } else if type == "circle" {
    return circle-box(
      prefix,
      marker-size
    )
  } else if type == "svg" {
    let svg-image = marker-config.at("svg", default: none)
    
    let position-rotation = {
      if position == "tl" { 0deg }
      else if position == "tr" { 90deg }
      else if position == "br" { 180deg }
      else if position == "bl" { 270deg }
      else { 0deg }
    }
    
    let total-rotation = position-rotation
    
    assert(svg-image != none, message: "SVG image must be provided")
    return gen-svg-box(
      prefix,
      rotate(total-rotation, svg-image),
      marker-size
    )
  } else {
    panic("Unknown marker type: " + type)
  }
}

// Place un marqueur dans un coin spécifique avec une marge
// - position: Position du coin (0% ou 100%)
// - marker-config: Configuration du marqueur
// - marker-size: Taille du marqueur
// - marker-margin: Marge entre le marqueur et le bord de la page
#let place-corner-marker(
  position-code,
  marker-config,
  marker-size,
  marker-margin: 0mm
) = {
  if marker-config == none { return }
  let marker = create-marker(position-code, marker-config, marker-size)
  if marker == none { return }
  
  let alignement = {
    if (position-code == "tl") {
      top+left
    } else if (position-code == "tr") {
      top+right
    } else if (position-code == "bl") {
      bottom+left
    } else if (position-code == "br") {
      bottom+right
    }
  }
  
  // Appliquer la marge en fonction de la position
  let dx = if position-code.contains("l") { marker-margin } else { -marker-margin }
  let dy = if position-code.contains("t") { marker-margin } else { -marker-margin }
  
  place(
    alignement,
    dx: dx,
    dy: dy,
    marker
  )
}

// Configuration des marqueurs dans chaque coin de la page
// - top/bottom-left/right-config: Configuration du marqueur pour le coin spécifié
// - marker-size: Taille par défaut des marqueurs
// - marker-margin: Marge entre les marqueurs et le bord de la page
#let setup-corner-markers(
  top-left-config: none,
  top-right-config: none,
  bottom-left-config: none,
  bottom-right-config: none,
  marker-size: 10mm,
  marker-margin: 0mm
) = {
  context if generating-content.get() {
    if top-left-config != none {
      place-corner-marker("tl", top-left-config, marker-size, marker-margin: marker-margin)
    }
    if top-right-config != none {
      place-corner-marker("tr", top-right-config, marker-size, marker-margin: marker-margin)
    }
    if bottom-left-config != none {
      place-corner-marker("bl", bottom-left-config, marker-size, marker-margin: marker-margin)
    }
    if bottom-right-config != none {
      place-corner-marker("br", bottom-right-config, marker-size, marker-margin: marker-margin)
    }
  }
}
