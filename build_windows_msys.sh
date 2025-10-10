#!/bin/bash

# QSF Windows Build Script for MSYS2
# This script sets up and builds QSF for Windows using MSYS2 native packages

set -e

echo "=========================================="
echo "QSF Windows Build Script for MSYS2"
echo "=========================================="

# Check if we're in MSYS environment
if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "❌ Error: This script must be run in MSYS2 MINGW64 environment"
    echo "   Please open MSYS2 MINGW64 terminal and run this script"
    exit 1
fi

echo "✅ Detected MSYS2 MINGW64 environment"

# Function to check if package is installed
check_package() {
    if pacman -Qi "$1" &>/dev/null; then
        echo "✅ $1 is installed"
        return 0
    else
        echo "❌ $1 is not installed"
        return 1
    fi
}

# Function to install package if not present
install_if_missing() {
    if ! check_package "$1"; then
        echo "📦 Installing $1..."
        pacman -S --noconfirm "$1"
    fi
}

echo ""
echo "🔍 Checking and installing dependencies..."

# Essential build tools
install_if_missing mingw-w64-x86_64-cmake
install_if_missing mingw-w64-x86_64-make
install_if_missing mingw-w64-x86_64-gcc
install_if_missing mingw-w64-x86_64-gcc-libs
install_if_missing mingw-w64-x86_64-pkg-config
install_if_missing git
install_if_missing mingw-w64-x86_64-python

# Qt5 for GUI miner
install_if_missing mingw-w64-x86_64-qt5-base
install_if_missing mingw-w64-x86_64-qt5-tools

# Required libraries
install_if_missing mingw-w64-x86_64-boost
install_if_missing mingw-w64-x86_64-openssl
install_if_missing mingw-w64-x86_64-zeromq
install_if_missing mingw-w64-x86_64-libiconv
install_if_missing mingw-w64-x86_64-expat
install_if_missing mingw-w64-x86_64-unbound
install_if_missing mingw-w64-x86_64-libsodium
install_if_missing mingw-w64-x86_64-hidapi
install_if_missing mingw-w64-x86_64-protobuf
install_if_missing mingw-w64-x86_64-libusb
install_if_missing mingw-w64-x86_64-readline
install_if_missing mingw-w64-x86_64-ncurses
install_if_missing mingw-w64-x86_64-icu

echo ""
echo "🏗️  Setting up build environment..."

# Create build directory
BUILD_DIR="build-windows"
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "📁 Build directory: $(pwd)"

# Configure with CMake
echo ""
echo "⚙️  Configuring with CMake..."

cmake .. -G "MSYS Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QUANTUM_SAFE_MINER=ON \
    -DBUILD_GUI_DEPS=ON \
    -DQSF_QUANTUM_SAFE_ENABLED=ON \
    -DARCH=x86-64 \
    -DMANUAL_SUBMODULES=1 \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DQt5_DIR=/mingw64/lib/cmake/Qt5 \
    -DCMAKE_PREFIX_PATH=/mingw64

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

echo "✅ CMake configuration successful!"

# Build the project
echo ""
echo "🔨 Building QSF..."

# Get number of CPU cores for parallel build
CORES=$(nproc)
echo "Using $CORES parallel jobs"

make -j"$CORES"

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"

# Check what was built
echo ""
echo "📋 Build results:"
if [ -d "bin" ]; then
    echo "📁 Built executables:"
    ls -la bin/*.exe 2>/dev/null || echo "No .exe files found"
    
    echo ""
    echo "📁 Built libraries:"
    ls -la bin/*.dll 2>/dev/null || echo "No .dll files found"
else
    echo "❌ No bin directory found"
fi

# Create distribution package
echo ""
echo "📦 Creating distribution package..."

DIST_DIR="distribution/QSF"
mkdir -p "$DIST_DIR"

# Copy executables
if [ -d "bin" ]; then
    cp bin/*.exe "$DIST_DIR/" 2>/dev/null || true
    cp bin/*.dll "$DIST_DIR/" 2>/dev/null || true
    
    # Copy Qt platform plugins if they exist
    if [ -d "bin/platforms" ]; then
        cp -r bin/platforms "$DIST_DIR/"
    fi
fi

# Copy documentation and config
cp ../README.md "$DIST_DIR/" 2>/dev/null || true
cp ../qsf.conf.example "$DIST_DIR/" 2>/dev/null || true

# Create batch files for easy execution
cat > "$DIST_DIR/run-gui-miner.bat" << 'EOF'
@echo off
cd /d "%~dp0"
echo Starting QSF GUI Miner...
qsf-gui-miner.exe
pause
EOF

cat > "$DIST_DIR/run-daemon.bat" << 'EOF'
@echo off
cd /d "%~dp0"
echo Starting QSF Daemon...
qsf.exe --rpc-bind-ip=127.0.0.1 --rpc-bind-port=18072
pause
EOF

cat > "$DIST_DIR/README-Windows.txt" << 'EOF'
QSF Windows Build
================

This is a Windows build of QSF (Quantum Safe Foundation) cryptocurrency.

Files:
- qsf.exe: The QSF daemon
- qsf-gui-miner.exe: The GUI mining application
- qsf-wallet-cli.exe: Command-line wallet
- *.dll: Required libraries

Quick Start:
1. Double-click "run-daemon.bat" to start the daemon
2. Double-click "run-gui-miner.bat" to start the GUI miner

For advanced usage, run the executables directly from command prompt.

Note: Make sure all .dll files are in the same directory as the executables.
EOF

echo "✅ Distribution package created in: $DIST_DIR"

# Test the build
echo ""
echo "🧪 Testing the build..."

if [ -f "bin/qsf-gui-miner.exe" ]; then
    echo "✅ GUI miner executable found"
    echo "   Size: $(du -h bin/qsf-gui-miner.exe | cut -f1)"
else
    echo "❌ GUI miner executable not found"
fi

if [ -f "bin/qsf.exe" ]; then
    echo "✅ Daemon executable found"
    echo "   Size: $(du -h bin/qsf.exe | cut -f1)"
else
    echo "❌ Daemon executable not found"
fi

echo ""
echo "🎉 Windows build completed successfully!"
echo ""
echo "📁 Output location: $(pwd)/$DIST_DIR"
echo "🚀 You can now copy the QSF folder to any Windows machine and run it!"
echo ""
echo "💡 Tips:"
echo "   - Test the GUI miner: ./bin/qsf-gui-miner.exe"
echo "   - Test the daemon: ./bin/qsf.exe --help"
echo "   - Distribution package is ready in: $DIST_DIR"
echo ""
echo "=========================================="
