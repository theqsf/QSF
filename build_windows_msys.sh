#!/bin/bash

# QSF Windows Build Script for MSYS2
# Updated version with automatic generator detection and proper compiler setup

set -e

echo "=========================================="
echo "QSF Windows Build Script for MSYS2"
echo "=========================================="

# --- Environment Check ---
if [[ "$MSYSTEM" != "MINGW64" && "$MSYSTEM" != "MSYS" ]]; then
    echo "❌ Error: This script must be run in MSYS2 (MINGW64 or MSYS) environment"
    echo "   Open MSYS2 MINGW64 terminal and run this script again."
    exit 1
fi

if [[ "$MSYSTEM" == "MINGW64" ]]; then
    echo "✅ Detected MSYS2 MINGW64 environment"
    CMAKE_GENERATOR="MinGW Makefiles"
    MAKE_CMD="mingw32-make"
else
    echo "✅ Detected MSYS2 (non-MINGW) environment"
    CMAKE_GENERATOR="MSYS Makefiles"
    MAKE_CMD="make"
fi

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
# Core tools
for pkg in mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-gcc \
           mingw-w64-x86_64-gcc-libs mingw-w64-x86_64-pkg-config git \
           mingw-w64-x86_64-python; do
    install_if_missing "$pkg"
done

# Qt5
for pkg in mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-tools; do
    install_if_missing "$pkg"
done

# Libraries
for pkg in mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq \
           mingw-w64-x86_64-libiconv mingw-w64-x86_64-expat mingw-w64-x86_64-unbound \
           mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi mingw-w64-x86_64-protobuf \
           mingw-w64-x86_64-libusb mingw-w64-x86_64-readline mingw-w64-x86_64-ncurses \
           mingw-w64-x86_64-icu; do
    install_if_missing "$pkg"
done

# --- Git Submodules ---
echo ""
echo "🔄 Checking and updating Git submodules..."
if [ -d ".git" ]; then
    git submodule sync --recursive
    git submodule update --init --recursive --force
    echo "✅ Submodules are up to date"
else
    echo "⚠️ Not a git repository. Skipping submodule update."
fi

# --- Build Setup ---
echo ""
echo "🏗️ Setting up build environment..."
BUILD_DIR="build-windows"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "📁 Build directory: $(pwd)"

# --- CMake Config ---
echo ""
echo "⚙️ Configuring with CMake..."
cmake .. -G "$CMAKE_GENERATOR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QUANTUM_SAFE_MINER=ON \
    -DBUILD_GUI_DEPS=ON \
    -DQSF_QUANTUM_SAFE_ENABLED=ON \
    -DARCH=x86-64 \
    -DMANUAL_SUBMODULES=1 \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_PREFIX_PATH=/mingw64 \
    -DQt5_DIR=/mingw64/lib/cmake/Qt5

echo "✅ CMake configuration successful!"

# --- Build ---
echo ""
echo "🔨 Building QSF..."
CORES=2
echo "Using $CORES parallel jobs"
$MAKE_CMD -j"$CORES" || { echo "❌ Build failed"; exit 1; }
echo "✅ Build successful!"

# --- Packaging ---
echo ""
echo "📦 Creating distribution package..."
DIST_DIR="distribution/QSF"
mkdir -p "$DIST_DIR"

# Copy binaries
if [ -d "bin" ]; then
    cp bin/*.exe "$DIST_DIR/" 2>/dev/null || true
    cp bin/*.dll "$DIST_DIR/" 2>/dev/null || true
    [ -d "bin/platforms" ] && cp -r bin/platforms "$DIST_DIR/"
    # Copy icon files for GUI miner
    cp bin/qsf_icon.ico "$DIST_DIR/" 2>/dev/null || true
    cp ../src/gui_miner/icons/qsf_icon.png "$DIST_DIR/" 2>/dev/null || true
fi

# Copy docs
cp ../README.md "$DIST_DIR/" 2>/dev/null || true
cp ../qsf.conf.example "$DIST_DIR/" 2>/dev/null || true

# Create run scripts
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
=================

Files:
- qsf.exe: Daemon
- qsf-gui-miner.exe: GUI Miner
- qsf-wallet-cli.exe: CLI Wallet
- *.dll: Required libraries

Quick Start:
1. run-daemon.bat
2. run-gui-miner.bat
EOF

echo "✅ Distribution package created: $DIST_DIR"

# --- Verify ---
echo ""
echo "🧪 Testing build..."
[[ -f "bin/qsf-gui-miner.exe" ]] && echo "✅ GUI miner found" || echo "❌ GUI miner missing"
[[ -f "bin/qsf.exe" ]] && echo "✅ Daemon found" || echo "❌ Daemon missing"

echo ""
echo "🎉 Build completed successfully!"
echo "📁 Output: $(pwd)/$DIST_DIR"
echo "=========================================="
