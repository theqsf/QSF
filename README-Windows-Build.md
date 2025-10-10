# QSF Windows Build Guide

This guide provides a reliable method to build QSF for Windows using MSYS2, bypassing the complex depends system that was causing issues.

## Prerequisites

1. **MSYS2 Installation**: Download and install MSYS2 from https://www.msys2.org/
2. **Open MSYS2 MINGW64 Terminal**: Use the MINGW64 terminal (not MSYS2)

## Quick Start

### Option 1: Automated Build (Recommended)

```bash
# 1. Setup environment (run once)
./setup_windows_environment.sh

# 2. Build QSF
./build_windows_msys.sh
```

### Option 2: Manual Build

```bash
# 1. Install dependencies
pacman -S --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-gcc-libs mingw-w64-x86_64-pkg-config git mingw-w64-x86_64-python mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-tools mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq mingw-w64-x86_64-libiconv mingw-w64-x86_64-expat mingw-w64-x86_64-unbound mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi mingw-w64-x86_64-protobuf mingw-w64-x86_64-libusb mingw-w64-x86_64-readline mingw-w64-x86_64-ncurses mingw-w64-x86_64-icu

# 2. Create build directory
mkdir build-windows && cd build-windows

# 3. Configure with CMake
cmake .. -G "MSYS Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QUANTUM_SAFE_MINER=ON \
    -DBUILD_GUI_DEPS=ON \
    -DQSF_QUANTUM_SAFE_ENABLED=ON \
    -DARCH=x86-64 \
    -DMANUAL_SUBMODULES=1 \
    -DCMAKE_CXX_STANDARD=17 \
    -DQt5_DIR=/mingw64/lib/cmake/Qt5

# 4. Build
make -j$(nproc)
```

## What This Approach Does

1. **Uses MSYS2 Native Packages**: Instead of building dependencies from source, we use MSYS2's pre-built packages
2. **Bypasses Depends System**: Avoids the complex contrib/depends system that was causing archiver interface issues
3. **Simplified Build Process**: Direct CMake configuration with MSYS2 packages
4. **Automatic Distribution**: Creates a ready-to-distribute package with batch files

## Expected Output

After successful build, you'll find:

```
build-windows/
├── bin/
│   ├── qsf.exe                    # QSF daemon
│   ├── qsf-gui-miner.exe          # GUI miner
│   ├── qsf-wallet-cli.exe         # Wallet CLI
│   ├── Qt5*.dll                   # Qt dependencies
│   └── lib*.dll                   # System dependencies
└── distribution/
    └── QSF/
        ├── qsf.exe
        ├── qsf-gui-miner.exe
        ├── qsf-wallet-cli.exe
        ├── *.dll
        ├── run-gui-miner.bat
        ├── run-daemon.bat
        └── README-Windows.txt
```

## Troubleshooting

### Common Issues

1. **"MSYS2 MINGW64 environment not detected"**
   - Make sure you're using the MINGW64 terminal, not MSYS2
   - Check that `$MSYSTEM` is set to `MINGW64`

2. **Package installation fails**
   - Update package database: `pacman -Sy`
   - Try installing packages individually

3. **CMake configuration fails**
   - Check that Qt5 is properly installed: `qmake -version`
   - Verify Qt5_DIR path: `/mingw64/lib/cmake/Qt5`

4. **Build fails with missing libraries**
   - Ensure all required packages are installed
   - Check that pkg-config can find libraries: `pkg-config --list-all | grep zmq`

### Manual Dependency Check

```bash
# Check Qt5
qmake -version

# Check CMake
cmake --version

# Check GCC
gcc --version

# Check ZMQ
pkg-config --modversion libzmq

# Check Boost
pkg-config --modversion boost
```

## Advantages of This Approach

1. **Reliability**: Uses well-tested MSYS2 packages
2. **Speed**: No need to compile dependencies from source
3. **Maintainability**: Easy to update and reproduce builds
4. **Compatibility**: Works with standard MSYS2/MinGW toolchain
5. **Distribution**: Creates ready-to-use Windows packages

## Alternative: Fix Depends System

If you prefer to use the original depends system, the archiver interface issue can be fixed by:

1. Adding proper archiver detection in the depends system
2. Using a different cross-compilation approach
3. Modifying the libiconv build configuration

However, the MSYS2 native package approach is recommended for reliability and ease of use.

## Support

If you encounter issues:

1. Check that you're in MSYS2 MINGW64 environment
2. Verify all dependencies are installed
3. Check the build logs for specific error messages
4. Ensure you have sufficient disk space and memory

The automated scripts should handle most common issues automatically.
