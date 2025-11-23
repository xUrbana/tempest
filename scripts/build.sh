#!/usr/bin/env bash
set -e

echo "Building with CMake..."
# Create a build directory if it doesn't exist
mkdir -p build

# Configure the project with CMake
cmake -B build -S .

# Build the project
cmake --build build -j8

