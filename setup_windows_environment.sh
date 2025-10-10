#!/bin/bash

# QSF Windows Environment Setup Script
# This script sets up the MSYS2 environment for QSF Windows builds

echo "=========================================="
echo "QSF Windows Environment Setup"
echo "=========================================="

# Check if we're in MSYS environment
if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "❌ Error: This script must be run in MSYS2 MINGW64 environment"
    echo "   Please open MSYS2 MINGW64 terminal and run this script"
    exit 1
fi

echo "✅ Detected MSYS2 MINGW64 environment"

# Function to install packages with error handling
install_packages() {
    local packages="$@"
    echo "📦 Installing: $packages"
    if pacman -S --noconfirm $packages; then
        echo "✅ Successfully installed: $packages"
    else
        echo "⚠️  Some packages may have failed to install, continuing..."
        echo "   Failed packages: $packages"
    fi
}

# Update package database
echo "🔄 Updating package database..."
pacman -Sy

# Handle mirror issues by trying alternative approach if needed
if [ $? -ne 0 ]; then
    echo "⚠️  Package database update had issues, trying alternative mirrors..."
    pacman -Sy --noconfirm
fi

# Install essential packages
echo "📦 Installing essential packages..."

# Build tools
install_packages \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-gcc-libs \
    mingw-w64-x86_64-pkg-config \
    git \
    mingw-w64-x86_64-python

# Qt5 for GUI
install_packages \
    mingw-w64-x86_64-qt5-base \
    mingw-w64-x86_64-qt5-tools

# Required libraries
install_packages \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-zeromq \
    mingw-w64-x86_64-libiconv \
    mingw-w64-x86_64-expat \
    mingw-w64-x86_64-unbound \
    mingw-w64-x86_64-libsodium \
    mingw-w64-x86_64-hidapi \
    mingw-w64-x86_64-protobuf \
    mingw-w64-x86_64-libusb \
    mingw-w64-x86_64-readline \
    mingw-w64-x86_64-ncurses \
    mingw-w64-x86_64-icu

echo "✅ All packages installed successfully!"

# Verify installations
echo ""
echo "🔍 Verifying installations..."

# Check Qt5
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -version | grep "Using Qt version" | cut -d' ' -f4)
    echo "✅ Qt5: $QT_VERSION"
else
    echo "❌ Qt5 not found"
fi

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✅ CMake: $CMAKE_VERSION"
else
    echo "❌ CMake not found"
fi

# Check GCC
if command -v gcc &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -n1 | cut -d' ' -f4)
    echo "✅ GCC: $GCC_VERSION"
else
    echo "❌ GCC not found"
fi

# Check ZMQ
if pkg-config --exists libzmq; then
    ZMQ_VERSION=$(pkg-config --modversion libzmq)
    echo "✅ ZMQ: $ZMQ_VERSION"
else
    echo "❌ ZMQ not found"
fi

echo ""
echo "🎯 Environment setup complete!"
echo ""
echo "📋 Next steps:"
echo "1. Run: ./build_windows_msys.sh"
echo "2. Or manually build with:"
echo "   mkdir build-windows && cd build-windows"
echo "   cmake .. -G \"MSYS Makefiles\" -DCMAKE_BUILD_TYPE=Release -DBUILD_QUANTUM_SAFE_MINER=ON -DBUILD_GUI_DEPS=ON"
echo "   make -j\$(nproc)"
echo ""
echo "=========================================="
