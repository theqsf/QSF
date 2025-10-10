#!/bin/bash

# QSF Windows Build Setup Script
# This script helps set up the Windows build environment and provides guidance

echo "=========================================="
echo "QSF Windows Build Setup"
echo "=========================================="
echo ""

# Check if we're in MSYS environment
if [[ "$MSYSTEM" == "MINGW64" || "$MSYSTEM" == "MINGW32" ]]; then
    echo "✅ Detected MSYS environment: $MSYSTEM"
else
    echo "⚠️  Warning: This script is designed for MSYS environment"
    echo "   Current environment: $MSYSTEM"
fi

echo ""
echo "🔧 Windows Build Configuration:"
echo "   - Target: Windows (MSYS cross-compilation)"
echo "   - GUI Miner: Enhanced daemon detection"
echo "   - Daemon: Standalone executable support"
echo ""

# Check for required tools
echo "🔍 Checking build dependencies..."

# Check for Qt5
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -version | grep "Using Qt version" | cut -d' ' -f4)
    echo "✅ Qt5 found: $QT_VERSION"
else
    echo "❌ Qt5 not found. Please install Qt5 development packages"
    echo "   pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-tools"
fi

# Check for CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✅ CMake found: $CMAKE_VERSION"
else
    echo "❌ CMake not found. Please install CMake"
    echo "   pacman -S mingw-w64-x86_64-cmake"
fi

# Check for ZMQ
if pkg-config --exists libzmq; then
    ZMQ_VERSION=$(pkg-config --modversion libzmq)
    echo "✅ ZMQ found: $ZMQ_VERSION"
else
    echo "❌ ZMQ not found. Please install ZMQ"
    echo "   pacman -S mingw-w64-x86_64-zeromq"
fi

echo ""
echo "📋 Build Instructions:"
echo "1. Create build directory:"
echo "   mkdir -p build-windows && cd build-windows"
echo ""
echo "2. Configure with CMake:"
echo "   cmake .. -G \"MSYS Makefiles\" \\"
echo "     -DCMAKE_BUILD_TYPE=Release \\"
echo "     -DBUILD_QUANTUM_SAFE_MINER=ON \\"
echo "     -DBUILD_GUI_DEPS=ON \\"
echo "     -DQSF_QUANTUM_SAFE_ENABLED=ON"
echo ""
echo "3. Build the project:"
echo "   make -j$(nproc)"
echo ""
echo "4. Test the build:"
echo "   cd bin"
echo "   ./qsf-gui-miner.exe"
echo ""

echo "🎯 Windows-Specific Features:"
echo "   ✅ Enhanced daemon path detection"
echo "   ✅ MSYS/MinGW path support"
echo "   ✅ Environment variable PATH scanning"
echo "   ✅ Graceful fallback to remote daemons"
echo "   ✅ Comprehensive error messages"
echo ""

echo "💡 Troubleshooting:"
echo "   - If daemon not found: Ensure qsf.exe is in the same directory as qsf-gui-miner.exe"
echo "   - For full features: Place qsf.exe in a standard Windows installation directory"
echo "   - Remote mining: GUI miner works with remote daemons even without local daemon"
echo ""

echo "📁 Expected Output Structure:"
echo "   build-windows/bin/"
echo "   ├── qsf.exe                    # QSF daemon"
echo "   ├── qsf-gui-miner.exe          # GUI miner"
echo "   ├── qsf-wallet-cli.exe         # Wallet CLI"
echo "   ├── Qt5*.dll                   # Qt dependencies"
echo "   ├── lib*.dll                   # System dependencies"
echo "   └── platforms/                 # Qt platform plugins"
echo ""

echo "=========================================="
echo "Setup complete! Ready for Windows build."
echo "=========================================="
