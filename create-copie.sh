#!/usr/bin/env bash

show_usage() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  --encoded-size N      : Size of encoded markers (default: 15)"
  echo "  --fiducial-size N     : Size of fiducial markers (default: 10)"
  echo "  --stroke-width N      : Width of marker stroke (default: 2)"
  echo "  --margin N            : Margin around markers (default: 3)"
  echo "  --copies N            : Number of copies to generate (default: 1)"
  echo "  --duplex N            : Duplex printing mode (0: single-sided, 1: double-sided) (default: 0)"
  echo "  --config N            : Marker configuration (1-10) (default: 10)"
  echo "  --grey-level N        : Grey level (0: black, 255: white) (default: 100)"
  echo "  --header-marker N     : Show header marker (0: hide, 1: show) (default: 1)"
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  show_usage
  exit 0
fi

./build-cmake/typst_interface "$@"
