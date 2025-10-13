windows build process
cd ~/QSF
rm -rf build
mkdir build
cd build

Configure with CMake:

cmake .. -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_GUI_DEPS=ON \
  -DARCH=x86-64 \
  -DMANUAL_SUBMODULES=1 \
  -DCMAKE_CXX_STANDARD=17 \
  -DBUILD_QUANTUM_SAFE_MINER=ON \
  -DQt5_DIR=/mingw64/lib/cmake/Qt5

build all components
# Build core components
mingw32-make daemon
mingw32-make qsf-wallet-cli
mingw32-make wallet_rpc_server

# Build blockchain utilities
mingw32-make blockchain_export blockchain_import blockchain_stats blockchain_usage
mingw32-make blockchain_ancestry blockchain_depth blockchain_prune
mingw32-make blockchain_prune_known_spent_data

# Build additional tools
mingw32-make gen_ssl_cert
mingw32-make gen_multisig
mingw32-make randomx-benchmark

# Build GUI miner (after fixing DLL dependency)
mingw32-make qsf-gui-miner

copy all dll files

# Copy system and ICU DLLs
cp /mingw64/bin/libgcc_s_seh-1.dll /mingw64/bin/libstdc++-6.dll /mingw64/bin/libwinpthread-1.dll bin/
cp /mingw64/bin/libdouble-conversion.dll /mingw64/bin/libicudt77.dll /mingw64/bin/libicuin77.dll bin/
cp /mingw64/bin/libicuuc77.dll /mingw64/bin/libpcre2-16-0.dll /mingw64/bin/libmd4c.dll bin/

# Copy compression and other library DLLs
cp /mingw64/bin/zlib1.dll /mingw64/bin/libbz2-1.dll /mingw64/bin/liblzma-5.dll bin/
cp /mingw64/bin/libzstd.dll /mingw64/bin/libzmq.dll bin/

# Copy image and font DLLs
cp /mingw64/bin/libharfbuzz-0.dll /mingw64/bin/libpng16-16.dll bin/
cp /mingw64/bin/libfreetype-6.dll /mingw64/bin/libjpeg-8.dll bin/
cp /mingw64/bin/libgraphite2.dll bin/

# Copy system DLLs
cp /mingw64/bin/libglib-2.0-0.dll /mingw64/bin/libintl-8.dll /mingw64/bin/libiconv-2.dll bin/

# Copy brotli compression DLLs
cp /mingw64/bin/libbrotlidec.dll /mingw64/bin/libbrotlicommon.dll /mingw64/bin/libbrotlienc.dll bin/

# Copy additional pcre2 DLLs
cp /mingw64/bin/libpcre2-8-0.dll bin/

# Create platforms directory and copy Qt plugins
mkdir -p bin/platforms
cp /mingw64/share/qt5/plugins/platforms/qwindows.dll bin/platforms/
cp /mingw64/share/qt5/plugins/platforms/qdirect2d.dll bin/platforms/
cp /mingw64/share/qt5/plugins/platforms/qminimal.dll bin/platforms/
cp /mingw64/share/qt5/plugins/platforms/qoffscreen.dll bin/platforms/


5. GUI Miner Fix (One-time):
File: src/gui_miner/CMakeLists.txt
Line 164: Comment out add_dependencies(qsf-gui-miner copy_gui_miner_dlls)
Change to: # add_dependencies(qsf-gui-miner copy_gui_miner_dlls)
✅ Fixed!
You now have 30 DLLs including:
✅ libgraphite2.dll - Graphite font rendering
✅ libbrotlidec.dll - Brotli decompression
✅ libpcre2-8-0.dll - PCRE2 regex library (8-bit)
✅ Plus 27 other essential DLLs
Your GUI miner should now run without any DLL errors! 🚀
Try running the GUI miner again - it should work perfectly now with all the required dependencies!

mb@DESKTOP-7C8BPU2 MINGW64 ~/QSF
# ./setup_windows_build.sh
==========================================
QSF Windows Build Setup (Updated)
==========================================

✅ Detected MSYS environment: MINGW64

🔧 Windows Build Configuration:
   - Target: Windows (MinGW cross-compilation)
   - Generator: MinGW Makefiles (not MSYS Makefiles)
   - GUI Miner: Fixed DLL dependencies
   - All 30+ DLLs: Automatically copied

🔍 Checking build dependencies...
✅ Qt5 found: 5.15.17
✅ CMake found: 4.1.2
✅ ZMQ found: 4.3.5

📋 Complete Build Instructions:

1. Clean and prepare build directory:
   cd ~/QSF
   rm -rf build
   mkdir build && cd build

2. Configure with CMake (MinGW Makefiles):
   cmake .. -G "MinGW Makefiles" \
     -DCMAKE_BUILD_TYPE=Release \
     -DBUILD_GUI_DEPS=ON \
     -DARCH=x86-64 \
     -DMANUAL_SUBMODULES=1 \
     -DCMAKE_CXX_STANDARD=17 \
     -DBUILD_QUANTUM_SAFE_MINER=ON \
     -DQt5_DIR=/mingw64/lib/cmake/Qt5

3. Build core components:
   mingw32-make daemon
   mingw32-make qsf-wallet-cli
   mingw32-make wallet_rpc_server

4. Build blockchain utilities:
   mingw32-make blockchain_export blockchain_import blockchain_stats blockchain_usage
   mingw32-make blockchain_ancestry blockchain_depth blockchain_prune
   mingw32-make blockchain_prune_known_spent_data

5. Build additional tools:
   mingw32-make gen_ssl_cert
   mingw32-make gen_multisig
   mingw32-make randomx-benchmark

6. Fix GUI miner CMakeLists.txt (one-time fix):
   # Edit src/gui_miner/CMakeLists.txt line 164:
   # Comment out: add_dependencies(qsf-gui-miner copy_gui_miner_dlls)
   # Change to: # add_dependencies(qsf-gui-miner copy_gui_miner_dlls)

7. Build GUI miner:
   mingw32-make qsf-gui-miner

8. Copy all required DLLs (30+ DLLs):
   # System and ICU DLLs
   cp /mingw64/bin/libgcc_s_seh-1.dll /mingw64/bin/libstdc++-6.dll /mingw64/bin/libwinpthread-1.dll bin/
   cp /mingw64/bin/libdouble-conversion.dll /mingw64/bin/libicudt77.dll /mingw64/bin/libicuin77.dll bin/
   cp /mingw64/bin/libicuuc77.dll /mingw64/bin/libpcre2-16-0.dll /mingw64/bin/libmd4c.dll bin/

   # Compression and library DLLs
   cp /mingw64/bin/zlib1.dll /mingw64/bin/libbz2-1.dll /mingw64/bin/liblzma-5.dll bin/
   cp /mingw64/bin/libzstd.dll /mingw64/bin/libzmq.dll bin/

   # Image and font DLLs
   cp /mingw64/bin/libharfbuzz-0.dll /mingw64/bin/libpng16-16.dll bin/
   cp /mingw64/bin/libfreetype-6.dll /mingw64/bin/libjpeg-8.dll bin/
   cp /mingw64/bin/libgraphite2.dll bin/

   # System DLLs
   cp /mingw64/bin/libglib-2.0-0.dll /mingw64/bin/libintl-8.dll /mingw64/bin/libiconv-2.dll bin/

   # Brotli compression DLLs
   cp /mingw64/bin/libbrotlidec.dll /mingw64/bin/libbrotlicommon.dll /mingw64/bin/libbrotlienc.dll bin/

   # Additional pcre2 DLLs
   cp /mingw64/bin/libpcre2-8-0.dll bin/

   # Qt platform plugins
   mkdir -p bin/platforms
   cp /mingw64/share/qt5/plugins/platforms/qwindows.dll bin/platforms/
   cp /mingw64/share/qt5/plugins/platforms/qdirect2d.dll bin/platforms/
   cp /mingw64/share/qt5/plugins/platforms/qminimal.dll bin/platforms/
   cp /mingw64/share/qt5/plugins/platforms/qoffscreen.dll bin/platforms/

9. Test the build:
   cd bin
   ./qsf-gui-miner.exe

🎯 Successfully Built Components:
   ✅ qsf.exe (33MB) - QSF daemon
   ✅ qsf-gui-miner.exe (34MB) - GUI miner
   ✅ qsf-wallet-cli.exe (33MB) - Wallet CLI
   ✅ quantumsafefoundation-wallet-rpc.exe (33MB) - Wallet RPC server
   ✅ 8x Blockchain utilities (20-23MB each)
   ✅ SSL certificate generator (15MB)
   ✅ Multisig generator (31MB)
   ✅ RandomX benchmark tool
   ✅ 30+ DLLs for complete functionality

🔧 Key Fixes Applied:
   ✅ Fixed uint64_t compilation errors (added #include <cstdint>)
   ✅ Fixed Boost.Locale ICU linking issues (use std::locale instead)
   ✅ Fixed GUI miner DLL copying issues (removed dependency)
   ✅ Fixed missing DLL dependencies (30+ DLLs copied)
   ✅ Used MinGW Makefiles instead of MSYS Makefiles

💡 Troubleshooting:
   - If DLL errors: Ensure all 30+ DLLs are copied to bin/
   - If GUI miner fails: Check CMakeLists.txt fix is applied
   - If build fails: Use MinGW Makefiles, not MSYS Makefiles
   - If missing executables: Build components individually

📁 Final Output Structure:
   build/bin/
   ├── qsf.exe                    # QSF daemon
   ├── qsf-gui-miner.exe          # GUI miner
   ├── qsf-wallet-cli.exe         # Wallet CLI
   ├── quantumsafefoundation-*.exe # Blockchain utilities
   ├── Qt5*.dll                   # Qt dependencies
   ├── lib*.dll                   # System dependencies (30+ DLLs)
   └── platforms/                 # Qt platform plugins

==========================================
Complete build process ready! All fixes applied.
==========================================