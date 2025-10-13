#!/bin/bash
set -e

echo "=========================================="
echo "🚀 QSF Windows Build Script (Streamlined)"
echo "=========================================="

# 🧭 Check environment
if [[ "$MSYSTEM" != "MINGW64" ]]; then
    echo "❌ Please run this script inside MSYS2 MINGW64 terminal."
    exit 1
fi
echo "✅ MSYS2 MINGW64 environment detected"

# 🧰 Required packages
packages=(
    mingw-w64-x86_64-cmake
    mingw-w64-x86_64-make
    mingw-w64-x86_64-gcc
    mingw-w64-x86_64-gcc-libs
    mingw-w64-x86_64-pkg-config
    mingw-w64-x86_64-python
    mingw-w64-x86_64-boost
    mingw-w64-x86_64-openssl
    mingw-w64-x86_64-zeromq
    mingw-w64-x86_64-libiconv
    mingw-w64-x86_64-expat
    mingw-w64-x86_64-unbound
    mingw-w64-x86_64-libsodium
    mingw-w64-x86_64-hidapi
    mingw-w64-x86_64-protobuf
    mingw-w64-x86_64-libusb
    mingw-w64-x86_64-readline
    mingw-w64-x86_64-ncurses
    mingw-w64-x86_64-icu
    mingw-w64-x86_64-qt5-base
    mingw-w64-x86_64-qt5-tools
    git
)

echo ""
echo "🔍 Checking dependencies..."
for pkg in "${packages[@]}"; do
    if ! pacman -Qi "$pkg" &>/dev/null; then
        echo "📦 Installing $pkg..."
        pacman -S --noconfirm "$pkg"
    else
        echo "✅ $pkg installed"
    fi
done

# 🧹 Prepare build directory
BUILD_DIR="build-windows"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "⚙️ Configuring with CMake..."
cmake .. -G "MSYS Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_GUI_DEPS=ON \
    -DQSF_QUANTUM_SAFE_ENABLED=ON \
    -DARCH=x86-64 \
    -DCMAKE_PREFIX_PATH=/mingw64 \
    -DQt5_DIR=/mingw64/lib/cmake/Qt5 \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON

echo ""
echo "🔨 Building QSF..."
CORES=$(nproc)
mingw32-make -j"$CORES"

# 🗃️ Create distribution directory
DIST_DIR="distribution/QSF"
mkdir -p "$DIST_DIR"
cp bin/*.exe "$DIST_DIR/" 2>/dev/null || true
cp bin/*.dll "$DIST_DIR/" 2>/dev/null || true
cp ../README.md "$DIST_DIR/" 2>/dev/null || true
cp ../qsf.conf.example "$DIST_DIR/" 2>/dev/null || true

# 🧩 Auto-copy required DLLs
echo ""
echo "🧩 Collecting DLL dependencies..."
cd "$DIST_DIR"

# Use ldd to find dependencies
for exe in *.exe; do
    echo "🔹 Scanning $exe..."
    for dll in $(ldd "$exe" | grep "=> /" | awk '{print $3}' | grep mingw64 | sort -u); do
        base=$(basename "$dll")
        if [ ! -f "$base" ]; then
            echo "   ↳ Copying $base"
            cp "$dll" .
        fi
    done
done

# 🖼️ Copy Qt platform plugins
mkdir -p platforms
cp /mingw64/share/qt5/plugins/platforms/qwindows.dll platforms/ 2>/dev/null || true

# 🧰 Create batch launchers
echo ""
echo "🧰 Creating Windows launchers..."
cat > run-daemon.bat <<'EOF'
@echo off
cd /d "%~dp0"
echo Starting QSF Daemon...
qsf.exe --rpc-bind-ip=127.0.0.1 --rpc-bind-port=18072
pause
EOF

cat > run-gui-miner.bat <<'EOF'
@echo off
cd /d "%~dp0"
echo Starting QSF GUI Miner...
qsf-gui-miner.exe
pause
EOF

# 🧾 Write readme
cat > README-Windows.txt <<'EOF'
==========================================
QuantumSafe (QSF) - Windows Build Package
==========================================

Contents:
- qsf.exe ................ Daemon
- qsf-gui-miner.exe ...... GUI Miner
- qsf-wallet-cli.exe ..... Command-line Wallet
- *.dll .................. Dependencies
- platforms\ ............. Qt platform plugin (required)

Usage:
1. Run "run-daemon.bat" to start the node
2. Run "run-gui-miner.bat" to start the miner
EOF

echo ""
echo "✅ Packaging complete!"
echo "📁 Output folder: $(pwd)"
echo "🎉 Your QSF Windows build is ready!"
echo "=========================================="
