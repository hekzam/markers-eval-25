#!/usr/bin/env bash

show_usage() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  --encoded-size N      : Size of encoded markers (default: 15)"
  echo "  --fiducial-size N     : Size of fiducial markers (default: 3)"
  echo "  --stroke-width N      : Width of marker stroke (default: 2)"
  echo "  --margin N            : Margin around markers (default: 3)"
  echo "  --copies N            : Number of copies to generate (default: 1)"
  echo "  --config N            : Marker configuration (default: 1)"
  echo "                          1: QR codes in all corners"
  echo "                          2: QR code only in bottom-right corner"
  echo "                          3: Circles in first three corners, QR code in bottom-right"
  echo "                          4: Circles on top, nothing in bottom-left, QR code in bottom-right"
  echo "                          5: Custom SVG markers in three corners, QR code in bottom-right"
  echo "                          6: ArUco markers, QR code in bottom-right"
  echo "                          7: Two ArUco markers, nothing in bottom-left, QR code in bottom-right"
  echo "                          8: Circle outlines in first three corners, QR code in bottom-right"
  echo "                          9: Squares in first three corners, QR code in bottom-right"
  echo "                          10: Square outlines in first three corners, QR code in bottom-right"
  echo "  --grey-level N        : Grey level (0: black, 255: white) (default: 0)"
  echo "  --header-size N       : Size of the header marker (default: 7)"
  echo "  --filename NAME       : Output filename (default: copy)"
  echo ""
  echo "Custom marker configuration options:"
  echo "  --top-left TYPE       : Marker type for top-left corner"
  echo "  --top-right TYPE      : Marker type for top-right corner"
  echo "  --bottom-left TYPE    : Marker type for bottom-left corner"
  echo "  --bottom-right TYPE   : Marker type for bottom-right corner"
  echo "  --header TYPE         : Marker type for header"
  echo ""
  echo "Marker TYPE format: type[:encoded][:outlined]"
  echo "  Available types: qrcode, datamatrix, aztec, pdf417-comp, rmqr, barcode, circle, square, aruco-svg, custom-svg"
  echo "  Example: qrcode:encoded - An encoded QR code"
  echo "  Example: circle:outlined - A circle outline (not filled)"
  echo "  Example: none - No marker"
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  show_usage
  exit 0
fi

./build-cmake/typst_interface "$@"
