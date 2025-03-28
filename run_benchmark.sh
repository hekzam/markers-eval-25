#!/usr/bin/env bash

show_help() {
    echo "Usage: $0 OUTPUT_DIR ATOMIC_BOXES IMAGE_DIR NB_COPIES"
    echo ""
    echo "Required arguments:"
    echo "  OUTPUT_DIR    Directory where output files will be saved"
    echo "  ATOMIC_BOXES  Path to the atomic boxes JSON file"
    echo "  IMAGE_DIR     Directory containing the images to process"
    echo "  NB_COPIES     Number of copies to generate with ARUCO_WITH_QR_BR configuration"
    echo ""
    echo "Example:"
    echo "  $0 ./output ./data/atomic_boxes.json ./data/images 5"
}

if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
    show_help
    exit 0
fi

if [ "$#" -ne 4 ]; then
    echo "Error: Incorrect number of arguments"
    show_help
    exit 1
fi

OUTPUT_DIR="$1"
ATOMIC_BOXES="$2"
IMAGE_DIR="$3"
NB_COPIES="$4"

if [ ! -f "$ATOMIC_BOXES" ]; then
    echo "Error: Atomic boxes file '$ATOMIC_BOXES' not found"
    exit 1
fi

if [ ! -d "$IMAGE_DIR" ]; then
    echo "Error: Image directory '$IMAGE_DIR' not found"
    exit 1
fi

if ! [[ "$NB_COPIES" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of copies must be a positive integer"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

echo "Running benchmark with:"
echo "  Output directory: $OUTPUT_DIR"
echo "  Atomic boxes: $ATOMIC_BOXES"
echo "  Image directory: $IMAGE_DIR"
echo "  Number of copies: $NB_COPIES"
echo ""

EXECUTABLE="./build-cmake/benchmark"

if [ ! -f "$EXECUTABLE" ]; then
    ALTERNATIVE_PATH="./src/benchmark"
    if [ -f "$ALTERNATIVE_PATH" ]; then
        EXECUTABLE="$ALTERNATIVE_PATH"
    else
        echo "Error: Benchmark executable not found in expected locations"
        echo "Please make sure the executable is built and specify the correct path in this script"
        exit 1
    fi
fi

"$EXECUTABLE" "$OUTPUT_DIR" "$ATOMIC_BOXES" "$IMAGE_DIR" "$NB_COPIES"

if [ $? -eq 0 ]; then
    echo "Benchmark completed successfully"
    echo "Output saved to: $OUTPUT_DIR"
else
    echo "Benchmark failed with exit code $?"
fi
