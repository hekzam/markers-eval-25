#!/bin/bash

# Display help message
show_help() {
    echo "Usage: $0 OUTPUT_DIR ATOMIC_BOXES IMAGE_DIR"
    echo ""
    echo "Required arguments:"
    echo "  OUTPUT_DIR    Directory where output files will be saved"
    echo "  ATOMIC_BOXES  Path to the atomic boxes JSON file"
    echo "  IMAGE_DIR     Directory containing the images to process"
    echo ""
    echo "Example:"
    echo "  $0 ./output ./data/atomic_boxes.json ./data/images"
}

# Check if help is requested
if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
    show_help
    exit 0
fi

# Check if correct number of arguments
if [ "$#" -ne 3 ]; then
    echo "Error: Incorrect number of arguments"
    show_help
    exit 1
fi

OUTPUT_DIR="$1"
ATOMIC_BOXES="$2"
IMAGE_DIR="$3"

# Check if files/directories exist
if [ ! -f "$ATOMIC_BOXES" ]; then
    echo "Error: Atomic boxes file '$ATOMIC_BOXES' not found"
    exit 1
fi

if [ ! -d "$IMAGE_DIR" ]; then
    echo "Error: Image directory '$IMAGE_DIR' not found"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Run the benchmark executable
echo "Running benchmark with:"
echo "  Output directory: $OUTPUT_DIR"
echo "  Atomic boxes: $ATOMIC_BOXES"
echo "  Image directory: $IMAGE_DIR"
echo ""

# Determine the path to the executable
# Assuming the executable is in the build directory
EXECUTABLE="./build-cmake/benchmark"

if [ ! -f "$EXECUTABLE" ]; then
    # Try to find the executable
    ALTERNATIVE_PATH="./src/benchmark"
    if [ -f "$ALTERNATIVE_PATH" ]; then
        EXECUTABLE="$ALTERNATIVE_PATH"
    else
        echo "Error: Benchmark executable not found in expected locations"
        echo "Please make sure the executable is built and specify the correct path in this script"
        exit 1
    fi
fi

# Run the benchmark
"$EXECUTABLE" "$OUTPUT_DIR" "$ATOMIC_BOXES" "$IMAGE_DIR"

# Check the exit status
if [ $? -eq 0 ]; then
    echo "Benchmark completed successfully"
    echo "Output saved to: $OUTPUT_DIR"
else
    echo "Benchmark failed with exit code $?"
fi
