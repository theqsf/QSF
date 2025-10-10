#!/bin/bash

# QSF Windows Clean Build Script
# This script cleans the build directory and rebuilds QSF

echo "=========================================="
echo "QSF Windows Clean Build"
echo "=========================================="

# Check if we're in MSYS environment
if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "❌ Error: This script must be run in MSYS2 MINGW64 environment"
    exit 1
fi

echo "✅ Detected MSYS2 MINGW64 environment"

# Clean build directory
BUILD_DIR="build-windows"
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "🏗️  Starting clean build..."

# Run the build script
./build_windows_msys.sh

echo "=========================================="
echo "Clean build completed!"
echo "=========================================="
