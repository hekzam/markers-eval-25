#!/usr/bin/env bash

show_usage() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  --encoded-size N      : Size of encoded markers (default: 15)"
  echo "  --unencoded-size N     : Size of unencoded markers (default: 3)"
  echo "  --header-size N       : Size of the header marker (default: 7)"
  echo "  --stroke-width N      : Width of marker stroke (default: 2)"
  echo "  --margin N            : Margin around markers (default: 3)"
  echo "  --grey-level N        : Grey level (0: black, 255: white) (default: 0)"
  echo "  --dpi N               : DPI (default: 300)"
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
  echo "  --filename NAME       : Output filename (default: copy)"
  echo ""
  echo "Custom marker configuration options:"
  echo "  --tl TYPE       : Marker type for top-left corner"
  echo "  --tr TYPE      : Marker type for top-right corner"
  echo "  --bl TYPE    : Marker type for bottom-left corner"
  echo "  --br TYPE   : Marker type for bottom-right corner"
  echo "  --header TYPE         : Marker type for header"
  echo ""
  echo "Marker TYPE format: type[:encoded][:outlined]"
  echo "  Available types: qrcode, micro-qr, datamatrix, aztec, pdf417, rmqr, code128, circle, square, triangle, aruco, qr-eye, custom"
  echo "  Example: qrcode:encoded - An encoded QR code"
  echo "  Example: circle:outlined - A circle outline (not filled)"
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  show_usage
  exit 0
fi

./build-cmake/typst_interface "$@"
