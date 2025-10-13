#!/bin/bash

# QSF Windows Build Script for MSYS2
# Streamlined version with automatic submodule handling

set -e

echo "=========================================="
echo "QSF Windows Build Script for MSYS2"
echo "=========================================="

# Check if we're in MSYS2 MINGW64 environment
if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "❌ Error: This script must be run in MSYS2 MINGW64 environment"
    echo "   Please open MSYS2 MINGW64 terminal and run this script"
    exit 1
fi
echo "✅ Detected MSYS2 MINGW64 environment"

# --- Dependency Installation ---
check_package() {
    pacman -Qi "$1" &>/dev/null && echo "✅ $1 is installed" || return 1
}

install_if_missing() {
    if ! check_package "$1"; then
        echo "📦 Installing $1..."
        pacman -S --noconfirm "$1"
    fi
}

echo ""
echo "🔍 Checking and installing dependencies..."
# Essential build tools
for pkg in mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-gcc \
           mingw-w64-x86_64-gcc-libs mingw-w64-x86_64-pkg-config git \
           mingw-w64-x86_64-python; do
    install_if_missing "$pkg"
done

# Qt5 for GUI miner
for pkg in mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-tools; do
    install_if_missing "$pkg"
done

# Required libraries
for pkg in mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq \
           mingw-w64-x86_64-libiconv mingw-w64-x86_64-expat mingw-w64-x86_64-unbound \
           mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi mingw-w64-x86_64-protobuf \
           mingw-w64-x86_64-libusb mingw-w64-x86_64-readline mingw-w64-x86_64-ncurses \
           mingw-w64-x86_64-icu; do
    install_if_missing "$pkg"
done

# --- Git Submodule Handling ---
echo ""
echo "🔄 Checking and updating Git submodules..."
if [ -d "../.git" ] || [ -d ".git" ]; then
    git submodule sync --recursive
    git submodule update --init --recursive --force
    echo "✅ Submodules are up to date"
else
    echo "⚠️ Not a git repository. Skipping submodule update."
fi

# --- Build Environment Setup ---
echo ""
echo "🏗️  Setting up build environment..."
BUILD_DIR="build-windows"
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "📁 Build directory: $(pwd)"

# --- CMake Configuration ---
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

echo "✅ CMake configuration successful!"

# --- Build Project ---
echo ""
echo "🔨 Building QSF..."
CORES=$(nproc)
echo "Using $CORES parallel jobs"
make -j"$CORES"
echo "✅ Build successful!"

# --- Build Output ---
echo ""
echo "📋 Build results:"
if [ -d "bin" ]; then
    echo "📁 Executables:"
    ls -la bin/*.exe 2>/dev/null || echo "No .exe files found"
    echo ""
    echo "📁 Libraries:"
    ls -la bin/*.dll 2>/dev/null || echo "No .dll files found"
else
    echo "❌ No bin directory found"
fi

# --- Distribution Packaging ---
echo ""
echo "📦 Creating distribution package..."
DIST_DIR="distribution/QSF"
mkdir -p "$DIST_DIR"

# Copy binaries and DLLs
if [ -d "bin" ]; then
    cp bin/*.exe "$DIST_DIR/" 2>/dev/null || true
    cp bin/*.dll "$DIST_DIR/" 2>/dev/null || true

    # Copy Qt platform plugins if exist
    if [ -d "bin/platforms" ]; then
        cp -r bin/platforms "$DIST_DIR/"
    fi
fi

# Copy docs/config
cp ../README.md "$DIST_DIR/" 2>/dev/null || true
cp ../qsf.conf.example "$DIST_DIR/" 2>/dev/null || true

# Batch files
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

Files:
- qsf.exe: Daemon
- qsf-gui-miner.exe: GUI miner
- qsf-wallet-cli.exe: CLI wallet
- *.dll: Required libraries

Quick Start:
1. run-daemon.bat
2. run-gui-miner.bat
EOF

echo "✅ Distribution package created in: $DIST_DIR"

# --- Build Test ---
echo ""
echo "🧪 Testing build..."
[[ -f "bin/qsf-gui-miner.exe" ]] && echo "✅ GUI miner found" || echo "❌ GUI miner missing"
[[ -f "bin/qsf.exe" ]] && echo "✅ Daemon found" || echo "❌ Daemon missing"

echo ""
echo "🎉 Windows build completed successfully!"
echo "📁 Output location: $(pwd)/$DIST_DIR"
echo "=========================================="
