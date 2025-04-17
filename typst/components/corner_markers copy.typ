#import "../common/global_variables.typ": generating-content
#import "../common/utils.typ": get-indexes, get-exam-id
#import "markers/barcode.typ": barcode, HEADER_MARKER_TYPE, CORNER_MARKER_TYPE
#import "./markers/circle.typ": circle-box
#import "./markers/svg_marker.typ": gen-svg-box
#import "./container.typ": gen-box
#import "../style/const.typ": get-encoded-marker-size, get-fiducial-marker-size, get-stroke-width, get-grey-level, get-header-marker-size, get-marker-margin

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
 * @param type Type de marqueur (ex: "qrcode")
 * @param marker-size Taille du marqueur
 * @param grey-level Niveau de gris du marqueur (0-255)
 */
#let create-tiaoma-barcode(prefix-position, type, marker-size, color) = {
  let (copy-i, page-i) = get-indexes()
  let barcode-data = (prefix-position, str(copy-i), str(page-i), get-exam-id())
  barcode(prefix-position, barcode-data, marker-size, type: type, color)
}

#let create-svg-marker(prefix-position, type) = {
  let svg-image = type
  let position-rotation = get-config(prefix-position).rotation
  assert(svg-image != none, message: "SVG image must be provided")
  gen-svg-box(
    prefix-position,
    rotate(position-rotation, svg-image),
    get-fiducial-marker-size()
  )
}

/**
 * Crée un marqueur à partir de sa configuration
 * @param prefix-position Préfixe de la position du marqueur
 * @param size Taille par défaut du marqueur
 */
#let create-corner-marker(prefix-position, type) = {
  if type == none { return none }
  let color = luma(get-grey-level())
  let fill-color = if type.contains("outline") { white } else { color }
  let stroke-width = if type.contains("outline") { 0 } else { get-stroke-width() }
  let encoded-marker-size = get-encoded-marker-size()
  let fiducial-marker-size = get-fiducial-marker-size()

  if CORNER_MARKER_TYPE.contains(type) {
    create-tiaoma-barcode(
      prefix-position,
      type,
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
  } else if type.contains("svg") {
    create-svg-marker(
      prefix-position,
      type)
  } else {
    panic("Unknown marker type: " + type)
  }
}

/**
 * Place un marqueur sur la page selon sa position
 * @param prefix-position Préfixe de la position du marqueur
 * @param marker Marqueur à placer
 * @param margin-x Marge horizontale
 * @param margin-y Marge verticale
 */
#let place-marker(prefix-position, marker, margin-x, margin-y) = {
  if marker == none { return }
  
  let position-config = get-config(prefix-position)
  let alignment = position-config.align
  
  let dx = position-config.dx-sign * margin-x
  let dy = position-config.dy-sign * margin-y
  
  place(
    alignment,
    dx: dx,
    dy: dy,
    marker
  )
}

/**
 * Place un marqueur d'en-tête au centre de la page
 * @param type Type de marqueur (ex: "rmqr", "barcode")
 */
#let place-header-marker(type) = {
  context if generating-content.get() {
    if type == none { return }
    
    let marker = create-tiaoma-barcode(
      PREFIX_TOP_CENTER,
      type,
      get-header-marker-size(),
      luma(get-grey-level()))
    
    place-marker(PREFIX_TOP_CENTER, marker, 0mm, get-marker-margin())
  }
}

/**
 * Place un marqueur dans un coin de la page
 * @param prefix-position Préfixe de la position du marqueur
 * @param type Type de marqueur (ex: "qrcode", "aztec")
 */
#let place-corner-marker(prefix-position, type) = {
  if type == none { return }
  let marker-margin = get-marker-margin()
  let marker = create-corner-marker(prefix-position, type)
  place-marker(prefix-position, marker, marker-margin, marker-margin)
}

/**
 * Place des marqueurs dans les coins de la page
 * @param config_key Clé de configuration (ex: "top-left-config") 
 * @param marker_config Configuration du marqueur pour ce coin
 */
#let setup-corner-markers(
  corner-ident,
  marker_config
) = {
  let CORNER_MAPPING = (
    "top-left": PREFIX_TOP_LEFT,
    "top-right": PREFIX_TOP_RIGHT,
    "bottom-left": PREFIX_BOTTOM_LEFT,
    "bottom-right": PREFIX_BOTTOM_RIGHT
  )
  context if generating-content.get() {
    let prefix_position = CORNER_MAPPING.at(corner-ident)
    
    if prefix_position != none and marker_config != none {
      place-corner-marker(
        prefix_position, 
        marker_config
      )
    }
  }
}
