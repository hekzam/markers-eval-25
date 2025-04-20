#import "../common/global_variables.typ": generating-content
#import "../common/utils.typ": get-indexes, get-exam-id
#import "markers/barcode.typ": barcode, HEADER_MARKER_TYPE, CORNER_MARKER_TYPE
#import "./markers/circle.typ": circle-box
#import "./markers/svg_marker.typ": gen-svg-box
#import "./container.typ": gen-box
#import "./markers/triangle.typ": triangle-box

// Images pour les marqueurs SVG
#let AR_1 = image("../assets/4x4_1000-190.svg")
#let AR_2 = image("../assets/4x4_1000-997.svg")
#let AR_3 = image("../assets/4x4_1000-999.svg")
#let CUSTOM_MARKER = image("../assets/marker-custom.svg")
#let QR_EYE = image("../assets/qr_eye.svg")
#let CROSS = image("../assets/cross.svg")

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
 * @param unencoded-marker-size Taille du marqueur non encodé
 * @param encoded-marker-size Taille du marqueur encodé
 * @param grey-level Niveau de gris du marqueur (0-255)
 */
#let create-tiaoma-barcode(prefix-position, type, unencoded-marker-size, encoded-marker-size, color) = {
  let (copy-i, page-i) = get-indexes()
  let barcode-data = (prefix-position, ..if type.contains("encoded") {
    (str(copy-i), str(page-i), get-exam-id())
  })
  let marker-size = if type.contains("encoded") {
    encoded-marker-size
  } else {
    unencoded-marker-size
  }
  barcode(prefix-position, barcode-data, marker-size, type: type, color)
}


#let create-svg-marker(prefix-position, type, marker-size) = {
  
  let svg-image = if type.contains("custom") {
    CUSTOM_MARKER
  } else if type.contains("aruco") and prefix-position == PREFIX_TOP_LEFT {
    AR_1
  } else if type.contains("aruco") and prefix-position == PREFIX_TOP_RIGHT {
    AR_2
  } else if type.contains("aruco") and prefix-position == PREFIX_BOTTOM_LEFT {
    AR_3
  } else if type.contains("qr-eye") {
    QR_EYE
  } else if type.contains("cross") {
    CROSS
  } else {
    none
  }

  let position-rotation = get-config(prefix-position).rotation
  assert(svg-image != none, message: "SVG image must be provided")
  gen-svg-box(
    prefix-position,
    rotate(position-rotation, svg-image),
    marker-size
  )
}

/**
 * Crée un marqueur à partir de sa configuration
 * @param prefix-position Préfixe de la position du marqueur
 * @param size Taille par défaut du marqueur
 */
#let create-corner-marker(prefix-position, type, style-params) = {
  if type == none { return none }
  let color = luma(style-params.grey_level)
  let fill-color = if type.contains("outline") { white } else { color }
  let stroke-width = if type.contains("outline") { style-params.stroke_width } else { 0mm }
  let encoded-marker-size = style-params.encoded_marker_size
  let unencoded-marker-size = style-params.unencoded_marker_size

  if type.contains("circle") {
    circle-box(
      prefix-position,
      unencoded-marker-size,
      fill-color: fill-color,
      stroke-width: stroke-width,
      stroke-color: color)
  } else if type.contains("square") {
    gen-box(
      prefix-position,
      unencoded-marker-size,
      unencoded-marker-size,
      fill-color: fill-color,
      stroke-width: stroke-width,
      stroke-color: color)
  } else if type.contains("triangle") {
    triangle-box(
      prefix-position,
      unencoded-marker-size,
      fill-color: fill-color,
      stroke-width: stroke-width,
      stroke-color: color)
  } else if type.contains("custom") or type.contains("aruco") or type.contains("qr-eye") or type.contains("cross") {
    create-svg-marker(
      prefix-position,
      type,
      unencoded-marker-size)
  } else {
    create-tiaoma-barcode(
      prefix-position,
      type,
      unencoded-marker-size,
      encoded-marker-size,
      color)
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
#let place-header-marker(type, style-params) = {
  context if generating-content.get() {
    if type == none { return }
    
    let marker = create-tiaoma-barcode(
      PREFIX_TOP_CENTER,
      type,
      style-params.header_marker_size,
      style-params.header_marker_size,
      luma(style-params.grey_level))
    
    place-marker(PREFIX_TOP_CENTER, marker, 0mm, style-params.marker_margin)
  }
}

/**
 * Place un marqueur dans un coin de la page
 * @param prefix-position Préfixe de la position du marqueur
 * @param type Type de marqueur (ex: "qrcode", "aztec")
 */
#let place-corner-marker(prefix-position, type, style-params) = {
  if type == none { return }
  let marker = create-corner-marker(prefix-position, type, style-params)
  place-marker(prefix-position, marker, style-params.marker_margin, style-params.marker_margin)
}

/**
 * Place des marqueurs dans les coins de la page
 * @param config_key Clé de configuration (ex: "top-left-config") 
 * @param marker_config Configuration du marqueur pour ce coin
 */
#let setup-corner-markers(
  corner-ident,
  marker_config,
  style-params
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
        marker_config,
        style-params
      )
    }
  }
}
