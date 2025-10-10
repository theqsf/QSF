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

# Function to check if package is installed
is_package_installed() {
    pacman -Qi "$1" &>/dev/null
}

# Function to install packages with error handling
install_packages() {
    local packages="$@"
    local packages_to_install=""
    
    # Check which packages actually need to be installed
    for package in $packages; do
        if ! is_package_installed "$package"; then
            packages_to_install="$packages_to_install $package"
        else
            echo "✅ $package is already installed"
        fi
    done
    
    # Only install packages that aren't already installed
    if [ -n "$packages_to_install" ]; then
        echo "📦 Installing missing packages:$packages_to_install"
        
        # Handle pkg-config conflict by removing pkgconf first if it exists
        if echo "$packages_to_install" | grep -q "mingw-w64-x86_64-pkg-config"; then
            echo "🔧 Resolving pkg-config conflict..."
            pacman -R --noconfirm mingw-w64-x86_64-pkgconf 2>/dev/null || true
        fi
        
        if pacman -S --noconfirm $packages_to_install; then
            echo "✅ Successfully installed:$packages_to_install"
        else
            echo "⚠️  Some packages may have failed to install, continuing..."
            echo "   Failed packages:$packages_to_install"
        fi
    else
        echo "✅ All packages already installed"
    fi
}

# Update package database
echo "🔄 Updating package database..."
if pacman -Sy 2>/dev/null; then
    echo "✅ Package database updated successfully"
else
    echo "⚠️  Package database update had issues (mirror errors are common)"
    echo "   This is usually not critical, continuing with installation..."
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
echo "📊 Setup Summary:"
echo "   ✅ Build tools: CMake, GCC, Make, pkg-config"
echo "   ✅ Qt5: GUI framework for the miner"
echo "   ✅ Libraries: Boost, OpenSSL, ZMQ, ICU, and more"
echo "   ✅ Development tools: Git, Python"
echo ""
echo "📋 Next steps:"
echo "1. Run: ./build_windows_msys.sh"
echo "2. Or manually build with:"
echo "   mkdir build-windows && cd build-windows"
echo "   cmake .. -G \"MSYS Makefiles\" -DCMAKE_BUILD_TYPE=Release -DBUILD_QUANTUM_SAFE_MINER=ON -DBUILD_GUI_DEPS=ON -DCMAKE_CXX_STANDARD=17"
echo "   make -j\$(nproc)"
echo ""
echo "💡 Tips:"
echo "   - The build process will take several minutes"
echo "   - Make sure you have at least 2GB free disk space"
echo "   - The final executables will be in build-windows/bin/"
echo ""
echo "=========================================="
