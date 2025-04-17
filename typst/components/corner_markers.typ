#import "../common/global_variables.typ": generating-content
#import "../common/utils.typ": get-indexes, get-exam-id
#import "markers/barcode.typ": barcode
#import "./markers/circle.typ": circle-box
#import "./markers/svg_marker.typ": gen-svg-box
#import "./container.typ": gen-box
#import "@preview/tiaoma:0.2.1"

#let PREFIX_TOP_LEFT = "hztl"
#let PREFIX_TOP_RIGHT = "hztr"
#let PREFIX_BOTTOM_LEFT = "hzbl"
#let PREFIX_BOTTOM_RIGHT = "hzbr"
#let PREFIX_TOP_CENTER = "hztc"

/**
 * Configuration des coins
 */
#let POSITION_DATA = (
  (PREFIX_TOP_LEFT):     (align: top + left,    rotation: 0deg,   dx-sign: 1,  dy-sign: 1),
  (PREFIX_TOP_RIGHT):    (align: top + right,   rotation: 90deg,  dx-sign: -1, dy-sign: 1),
  (PREFIX_BOTTOM_LEFT):  (align: bottom + left,  rotation: 270deg, dx-sign: 1,  dy-sign: -1),
  (PREFIX_BOTTOM_RIGHT): (align: bottom + right, rotation: 180deg, dx-sign: -1, dy-sign: -1),
  (PREFIX_TOP_CENTER):   (align: top + center,  rotation: 0deg,   dx-sign: 0,  dy-sign: 1)
)

/**
 * Récupère la configuration d'un coin
 * @param prefix-position Préfixe de la position du marqueur
 */
#let get-config(prefix-position) = POSITION_DATA.at(
  prefix-position, 
  default: (align: center, rotation: 0deg, dx-sign: 1, dy-sign: 1)
)

/**
 * Crée un marqueur de type Tiaoma carré
 * @param prefix-position Préfixe de la position du marqueur
 * @param marker-config Configuration du marqueur
 * @param encoded-marker-size Taille du marqueur encodé
 * @param fiducial-marker-size Taille du marqueur fiducial
 * @param color Couleur du marqueur
 */
#let create-tiaoma-sqr-marker(prefix-position, marker-config, encoded-marker-size, fiducial-marker-size, color) = {
  let type = marker-config.at("type", default: "qrcode")
  let is-data-encoded = marker-config.at("is-data-encoded", default: false)
  let (copy-i, page-i) = get-indexes()
  let barcode-data = (prefix-position, ..if is-data-encoded {
    (str(copy-i), str(page-i), get-exam-id())
  })
  let marker-size = if is-data-encoded {encoded-marker-size} else {fiducial-marker-size}
  barcode(prefix-position, barcode-data, marker-size, type: type, color)
}

/**
 * Crée un marqueur de type Tiaoma rectangle (code-barre, QRcode rectangulaire etc.)
 * @param prefix-position Préfixe de la position du marqueur
 * @param type Type de marqueur (ex: "barcode")
 * @param maker-height Hauteur du marqueur
 * @param grey-level Niveau de gris du marqueur (0-255)
 */
#let create-tiaoma-rect-marker(prefix-position, type, maker-height, grey-level) = {
  let color = luma(grey-level)
  let (copy-i, page-i) = get-indexes()
  let barcode-data = (prefix-position, str(copy-i), str(page-i), get-exam-id())
  barcode(prefix-position, barcode-data, maker-height, type: type, color)
}

/**
 * Crée un marqueur SVG à partir de sa configuration
 * @param prefix-position Préfixe de la position du marqueur
 * @param marker-config Configuration du marqueur
 * @param fiducial-marker-size Taille du marqueur fiducial
 */
#let create-svg-marker(prefix-position, marker-config, fiducial-marker-size) = {
  let svg-image = marker-config.at("svg", default: none)
  let position-rotation = get-config(prefix-position).rotation
  assert(svg-image != none, message: "SVG image must be provided")
  gen-svg-box(
    prefix-position,
    rotate(position-rotation, svg-image),
    fiducial-marker-size
  )
}

/**
 * Crée un marqueur à partir de sa configuration
 * @param prefix-position Préfixe de la position du marqueur
 * @param marker-config Configuration du marqueur
 * @param size Taille par défaut du marqueur
 */
#let create-corner-marker(prefix-position, marker-config, encoded-marker-size,
  fiducial-marker-size, stroke-width, grey-level) = {
  let color = luma(grey-level)
  if marker-config == none { return none }
  let type = marker-config.at("type", default: "qrcode")
  let fill-color = if type.contains("outline") { white } else { color }

  if type == "qrcode" or type == "datamatrix" or type == "aztec" or type == "pdf417-comp" {
    create-tiaoma-sqr-marker(
      prefix-position,
      marker-config,
      encoded-marker-size,
      fiducial-marker-size,
      color)
  } else if type.contains("circle") {
    circle-box(
      prefix-position,
      fiducial-marker-size,
      fill-color: fill-color,
      stroke-width: stroke-width,
      stroke-color: color)
  } else if type.contains("square") {
    gen-box(
      prefix-position,
      fiducial-marker-size,
      fiducial-marker-size,
      fill-color: fill-color,
      stroke-width: stroke-width,
      stroke-color: color)
  } else if type == "svg" {
    create-svg-marker(
      prefix-position,
      marker-config,
      fiducial-marker-size)
  } else {
    panic("Unknown marker type: " + type)
  }
}

/**
 * Place un marqueur d'entête au centre haut de la page
 * @param width Largeur du marqueur
 * @param height Hauteur du marqueur
 * @param margin Marge entre le marqueur et le bord de la page
 * @param grey_level Niveau de gris du marqueur (0-255)
 */
#let place-header-marker(
  height,
  margin,
  grey-level
) = {
  context if generating-content.get() {
    
    let marker = create-tiaoma-rect-marker(
      PREFIX_TOP_CENTER,
      "pdf417-comp",
      height,
      grey-level)

    if marker == none { return }
    
    let position_config = get-config(PREFIX_TOP_CENTER)
    let alignment = position_config.align
    
    let dy = position_config.dy-sign * margin
    let dx = position_config.dx-sign * 0mm
    
    place(
      alignment,
      dy: dy,
      dx: dx,
      marker
    )
  }
}

/**
 * Place un marqueur dans un coin spécifique avec une marge
 * @param prefix-position Préfixe de la position du marqueur
 * @param marker-config Configuration du marqueur
 * @param marker-size Taille du marqueur
 * @param marker-margin Marge entre le marqueur et le bord de la page
 */
#let place-corner-marker(
  prefix-position,
  marker-config,
  encoded-marker-size,
  fiducial-marker-size,
  stroke-width,
  marker-margin,
  grey-level
) = {
  if marker-config == none { return }
  
  let marker = create-corner-marker(
    prefix-position, 
    marker-config, 
    encoded-marker-size, 
    fiducial-marker-size, 
    stroke-width, 
    grey-level)
  if marker == none { return }
  
  let corner-config = get-config(prefix-position)
  let alignment = corner-config.align
  
  let dx = corner-config.dx-sign * marker-margin
  let dy = corner-config.dy-sign * marker-margin
  
  place(
    alignment,
    dx: dx,
    dy: dy,
    marker
  )
}

/**
 * Place des marqueurs dans les coins de la page
 * @param config_key Clé de configuration (ex: "top-left-config") 
 * @param marker_config Configuration du marqueur pour ce coin
 * @param encoded_marker_size Taille des marqueurs encodés
 * @param fiducial_marker_size Taille des marqueurs fiduciaires
 * @param marker_margin Marge entre les marqueurs et le bord de la page
 */
#let setup-corner-markers(
  config_key,
  marker_config,
  encoded_marker_size,
  fiducial_marker_size,
  stroke-width,
  marker_margin,
  grey_level
) = {
  let CORNER_MAPPING = (
    "top-left": PREFIX_TOP_LEFT,
    "top-right": PREFIX_TOP_RIGHT,
    "bottom-left": PREFIX_BOTTOM_LEFT,
    "bottom-right": PREFIX_BOTTOM_RIGHT
  )
  context if generating-content.get() {
    let corner_label = config_key.slice(0, -7)
    let prefix_position = CORNER_MAPPING.at(corner_label, default: none)
    
    if prefix_position != none and marker_config != none {
      place-corner-marker(
        prefix_position, 
        marker_config, 
        encoded_marker_size, 
        fiducial_marker_size,
        stroke-width,
        marker_margin,
        grey_level
      )
    }
  }
}
