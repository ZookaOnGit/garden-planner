#!/usr/bin/env bash

# Exit on error
set -e

PROJECT_DIR="$(pwd)"
BUILD_DIR="$PROJECT_DIR/build"

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with Ninja and Qt6
echo "Configuring project with CMake..."
cmake .. -G Ninja

# Build
echo "Building project with Ninja..."
ninja

# Run the app
echo "Build complete. Running the app with:"
echo "$BUILD_DIR/garden-planner"
