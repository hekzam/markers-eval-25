#import "src/gen_copy.typ": gen-copies

// Images pour les marqueurs SVG
#let AR_1 = image("assets/4x4_1000-190.svg")
#let AR_2 = image("assets/4x4_1000-997.svg")
#let AR_3 = image("assets/4x4_1000-999.svg")
#let CUSTOM_MARKER = image("assets/marker-custom.svg")

/**
 * Génère une configuration standard pour un marqueur.
 * @param {string} type - Type de marqueur ("qrcode", "circle", "svg")
 * @param {bool} is_data_encoded - Si true, des données supplémentaires sont encodées
 * @param {image} svg_image - L'image SVG à utiliser (obligatoire si type="svg")
 * @return {dict} - Configuration du marqueur
 */
#let create-marker-config(type, is_data_encoded: false, svg_image: none) = {
  let config = (
    type: type,
    is-data-encoded: is_data_encoded
  )
  
  if type == "svg" {
    assert(svg_image != none, message: "Une image SVG doit être fournie pour le type 'svg'")
    config.insert("svg", svg_image)
  }
  
  return config
}

/**
 * Génère une configuration complète avec les marqueurs aux quatre coins.
 * @param {dict} top_left - Configuration du marqueur en haut à gauche
 * @param {dict} top_right - Configuration du marqueur en haut à droite
 * @param {dict} bottom_left - Configuration du marqueur en bas à gauche
 * @param {dict} bottom_right - Configuration du marqueur en bas à droite
 * @return {dict} - Configuration complète des marqueurs
 */
#let create-full-marker-config(top_left, top_right, bottom_left, bottom_right) = {
  return (
    top-left: top_left,
    top-right: top_right,
    bottom-left: bottom_left,
    bottom-right: bottom_right
  )
}

/**
 * Génère une configuration de marqueurs uniforme pour tous les coins.
 * @param {string} type - Type de marqueur pour tous les coins
 * @param {bool} is_data_encoded - Si true, des données supplémentaires sont encodées
 * @param {bool} only_bottom_right_encoded - Si true, seul le coin en bas à droite encode des données
 * @return {dict} - Configuration uniforme des marqueurs
 */
#let create-uniform-config(type, is_data_encoded: false, only_bottom_right_encoded: false) = {
  let base_config = create-marker-config(type, is_data_encoded: if only_bottom_right_encoded { false } else { is_data_encoded })
  let br_config = if only_bottom_right_encoded {
    create-marker-config(type, is_data_encoded: true)
  } else {
    base_config
  }
  
  return create-full-marker-config(base_config, base_config, base_config, br_config)
}

#let marker_configs = (
  // Config 1: QR codes avec données encodées dans tous les coins
  create-uniform-config("qrcode", is_data_encoded: true),
  
  // Config 2: QR codes avec données encodées uniquement dans le coin bas-droit
  create-uniform-config("qrcode", only_bottom_right_encoded: true),
  
  // Config 3: Cercles dans les trois premiers coins, QR code avec données dans le coin bas-droit
  create-full-marker-config(
    create-marker-config("circle"),
    create-marker-config("circle"),
    create-marker-config("circle"),
    create-marker-config("qrcode", is_data_encoded: true)
  ),
  
  // Config 4: Cercles en haut, rien en bas-gauche, QR code avec données en bas-droit
  create-full-marker-config(
    create-marker-config("circle"),
    create-marker-config("circle"),
    {},
    create-marker-config("qrcode", is_data_encoded: true)
  ),
  
  // Config 5: Marqueurs SVG personnalisés dans trois coins, QR code avec données en bas-droit
  create-full-marker-config(
    create-marker-config("svg", svg_image: CUSTOM_MARKER),
    create-marker-config("svg", svg_image: CUSTOM_MARKER),
    create-marker-config("svg", svg_image: CUSTOM_MARKER),
    create-marker-config("qrcode", is_data_encoded: true)
  ),
  
  // Config 6: Différents marqueurs ArUco, QR code avec données en bas-droit
  create-full-marker-config(
    create-marker-config("svg", svg_image: AR_1),
    create-marker-config("svg", svg_image: AR_2),
    create-marker-config("svg", svg_image: AR_3),
    create-marker-config("qrcode", is_data_encoded: true)
  ),
  
  // Config 7: Deux marqueurs ArUco, rien en bas-gauche, QR code avec données en bas-droit
  create-full-marker-config(
    create-marker-config("svg", svg_image: AR_1),
    create-marker-config("svg", svg_image: AR_2),
    {},
    create-marker-config("qrcode", is_data_encoded: true)
  ),
)

/**
 * Sélectionne une configuration de marqueur selon un index.
 * @param {string} config_index - Indice de la configuration à utiliser
 * @return {dict} - Configuration de marqueur correspondante
 */
#let get-marker-config(config_index) = {
  let idx = int(config_index)
  if idx >= 1 and idx <= marker_configs.len() {
    return marker_configs.at(idx - 1)
  } else {
    return marker_configs.last()
  }
}

// Paramètres pour la génération de copies
#let e-m-s = float(sys.inputs.at("encoded-marker-size", default: "15")) * 1mm
#let f-m-s = float(sys.inputs.at("fiducial-marker-size", default: "5")) * 1mm
#let m-m = float(sys.inputs.at("marker-margin", default: "3")) * 1mm
#let n-c = int(sys.inputs.at("nb-copies", default: "1"))
#let d-p = int(sys.inputs.at("duplex-printing", default: "0")) == 1
#let m-c = sys.inputs.at("marker-config", default: "2")

#gen-copies(
  "Le pire examen de tous les temps !", 
  nb-copies: n-c, 
  duplex-printing: d-p,
  get-marker-config(m-c),
  e-m-s,
  f-m-s,
  m-m
)