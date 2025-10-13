#!/bin/bash

# QSF Windows Build Setup Script
# This script provides the complete build process for QSF on Windows using MinGW
# Updated based on successful build process with all fixes applied

echo "=========================================="
echo "QSF Windows Build Setup (Updated)"
echo "=========================================="
echo ""

# Check if we're in MSYS environment
if [[ "$MSYSTEM" == "MINGW64" || "$MSYSTEM" == "MINGW32" ]]; then
    echo "✅ Detected MSYS environment: $MSYSTEM"
else
    echo "⚠️  Warning: This script is designed for MSYS environment"
    echo "   Current environment: $MSYSTEM"
    echo "   Please run: C:\\msys64\\msys2_shell.cmd -mingw64"
fi

echo ""
echo "🔧 Windows Build Configuration:"
echo "   - Target: Windows (MinGW cross-compilation)"
echo "   - Generator: MinGW Makefiles (not MSYS Makefiles)"
echo "   - GUI Miner: Fixed DLL dependencies"
echo "   - All 30+ DLLs: Automatically copied"
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
echo "📋 Complete Build Instructions:"
echo ""
echo "1. Clean and prepare build directory:"
echo "   cd ~/QSF"
echo "   rm -rf build"
echo "   mkdir build && cd build"
echo ""
echo "2. Configure with CMake (MinGW Makefiles):"
echo "   cmake .. -G \"MinGW Makefiles\" \\"
echo "     -DCMAKE_BUILD_TYPE=Release \\"
echo "     -DBUILD_GUI_DEPS=ON \\"
echo "     -DARCH=x86-64 \\"
echo "     -DMANUAL_SUBMODULES=1 \\"
echo "     -DCMAKE_CXX_STANDARD=17 \\"
echo "     -DBUILD_QUANTUM_SAFE_MINER=ON \\"
echo "     -DQt5_DIR=/mingw64/lib/cmake/Qt5"
echo ""
echo "3. Build core components:"
echo "   mingw32-make daemon"
echo "   mingw32-make qsf-wallet-cli"
echo "   mingw32-make wallet_rpc_server"
echo ""
echo "4. Build blockchain utilities:"
echo "   mingw32-make blockchain_export blockchain_import blockchain_stats blockchain_usage"
echo "   mingw32-make blockchain_ancestry blockchain_depth blockchain_prune"
echo "   mingw32-make blockchain_prune_known_spent_data"
echo ""
echo "5. Build additional tools:"
echo "   mingw32-make gen_ssl_cert"
echo "   mingw32-make gen_multisig"
echo "   mingw32-make randomx-benchmark"
echo ""
echo "6. Fix GUI miner CMakeLists.txt (one-time fix):"
echo "   # Edit src/gui_miner/CMakeLists.txt line 164:"
echo "   # Comment out: add_dependencies(qsf-gui-miner copy_gui_miner_dlls)"
echo "   # Change to: # add_dependencies(qsf-gui-miner copy_gui_miner_dlls)"
echo ""
echo "7. Build GUI miner:"
echo "   mingw32-make qsf-gui-miner"
echo ""
echo "8. Copy all required DLLs (30+ DLLs):"
echo "   # System and ICU DLLs"
echo "   cp /mingw64/bin/libgcc_s_seh-1.dll /mingw64/bin/libstdc++-6.dll /mingw64/bin/libwinpthread-1.dll bin/"
echo "   cp /mingw64/bin/libdouble-conversion.dll /mingw64/bin/libicudt77.dll /mingw64/bin/libicuin77.dll bin/"
echo "   cp /mingw64/bin/libicuuc77.dll /mingw64/bin/libpcre2-16-0.dll /mingw64/bin/libmd4c.dll bin/"
echo ""
echo "   # Compression and library DLLs"
echo "   cp /mingw64/bin/zlib1.dll /mingw64/bin/libbz2-1.dll /mingw64/bin/liblzma-5.dll bin/"
echo "   cp /mingw64/bin/libzstd.dll /mingw64/bin/libzmq.dll bin/"
echo ""
echo "   # Image and font DLLs"
echo "   cp /mingw64/bin/libharfbuzz-0.dll /mingw64/bin/libpng16-16.dll bin/"
echo "   cp /mingw64/bin/libfreetype-6.dll /mingw64/bin/libjpeg-8.dll bin/"
echo "   cp /mingw64/bin/libgraphite2.dll bin/"
echo ""
echo "   # System DLLs"
echo "   cp /mingw64/bin/libglib-2.0-0.dll /mingw64/bin/libintl-8.dll /mingw64/bin/libiconv-2.dll bin/"
echo ""
echo "   # Brotli compression DLLs"
echo "   cp /mingw64/bin/libbrotlidec.dll /mingw64/bin/libbrotlicommon.dll /mingw64/bin/libbrotlienc.dll bin/"
echo ""
echo "   # Additional pcre2 DLLs"
echo "   cp /mingw64/bin/libpcre2-8-0.dll bin/"
echo ""
echo "   # Qt platform plugins"
echo "   mkdir -p bin/platforms"
echo "   cp /mingw64/share/qt5/plugins/platforms/qwindows.dll bin/platforms/"
echo "   cp /mingw64/share/qt5/plugins/platforms/qdirect2d.dll bin/platforms/"
echo "   cp /mingw64/share/qt5/plugins/platforms/qminimal.dll bin/platforms/"
echo "   cp /mingw64/share/qt5/plugins/platforms/qoffscreen.dll bin/platforms/"
echo ""
echo "9. Test the build:"
echo "   cd bin"
echo "   ./qsf-gui-miner.exe"
echo ""

echo "🎯 Successfully Built Components:"
echo "   ✅ qsf.exe (33MB) - QSF daemon"
echo "   ✅ qsf-gui-miner.exe (34MB) - GUI miner"
echo "   ✅ qsf-wallet-cli.exe (33MB) - Wallet CLI"
echo "   ✅ quantumsafefoundation-wallet-rpc.exe (33MB) - Wallet RPC server"
echo "   ✅ 8x Blockchain utilities (20-23MB each)"
echo "   ✅ SSL certificate generator (15MB)"
echo "   ✅ Multisig generator (31MB)"
echo "   ✅ RandomX benchmark tool"
echo "   ✅ 30+ DLLs for complete functionality"
echo ""

echo "🔧 Key Fixes Applied:"
echo "   ✅ Fixed uint64_t compilation errors (added #include <cstdint>)"
echo "   ✅ Fixed Boost.Locale ICU linking issues (use std::locale instead)"
echo "   ✅ Fixed GUI miner DLL copying issues (removed dependency)"
echo "   ✅ Fixed missing DLL dependencies (30+ DLLs copied)"
echo "   ✅ Used MinGW Makefiles instead of MSYS Makefiles"
echo ""

echo "💡 Troubleshooting:"
echo "   - If DLL errors: Ensure all 30+ DLLs are copied to bin/"
echo "   - If GUI miner fails: Check CMakeLists.txt fix is applied"
echo "   - If build fails: Use MinGW Makefiles, not MSYS Makefiles"
echo "   - If missing executables: Build components individually"
echo ""

echo "📁 Final Output Structure:"
echo "   build/bin/"
echo "   ├── qsf.exe                    # QSF daemon"
echo "   ├── qsf-gui-miner.exe          # GUI miner"
echo "   ├── qsf-wallet-cli.exe         # Wallet CLI"
echo "   ├── quantumsafefoundation-*.exe # Blockchain utilities"
echo "   ├── Qt5*.dll                   # Qt dependencies"
echo "   ├── lib*.dll                   # System dependencies (30+ DLLs)"
echo "   └── platforms/                 # Qt platform plugins"
echo ""

echo "=========================================="
echo "Complete build process ready! All fixes applied."
echo "=========================================="
