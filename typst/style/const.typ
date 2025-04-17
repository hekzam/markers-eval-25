#let PAGE_WIDTH = 210mm
#let PAGE_HEIGHT = 297mm
#let MARGIN_X = 10mm
#let MARGIN_Y = 20mm

#let _encoded_marker_size = 15mm
#let _fiducial_marker_size = 3mm
#let _header_marker_size = 7mm
#let _stroke_width = 2mm
#let _marker_margin = 3mm
#let _grey_level = 0

#let get-encoded-marker-size() = _encoded_marker_size
#let get-fiducial-marker-size() = _fiducial_marker_size
#let get-header-marker-size() = _header_marker_size
#let get-stroke-width() = _stroke_width
#let get-marker-margin() = _marker_margin
#let get-grey-level() = _grey_level

#let set-encoded-marker-size(value) = {
  _encoded_marker_size = value
}

#let set-fiducial-marker-size(value) = {
  _fiducial_marker_size = value
}

#let set-header-marker-size(value) = {
  _header_marker_size = value
}

#let set-stroke-width(value) = {
  _stroke_width = value
}

#let set-marker-margin(value) = {
  _marker_margin = value
}

#let set-grey-level(value) = {
  _grey_level = value
}

// Initialize parameters from system inputs
#let init-style-params() = {
  set-encoded-marker-size(float(sys.inputs.at("encoded-marker-size", default: "15")) * 1mm)
  set-fiducial-marker-size(float(sys.inputs.at("fiducial-marker-size", default: "3")) * 1mm)
  set-header-marker-size(float(sys.inputs.at("header-marker-size", default: "7")) * 1mm)
  set-stroke-width(float(sys.inputs.at("stroke-width", default: "2")) * 1mm)
  set-marker-margin(float(sys.inputs.at("marker-margin", default: "3")) * 1mm)
  set-grey-level(int(sys.inputs.at("grey-level", default: "0")))
}