#!/bin/bash

# QSF Quantum-Safe Miner - Unified Build Script
# This script provides a simplified build process for all QSF components

set -e

echo "=========================================="
echo "QSF Quantum-Safe Miner - Unified Build"
echo "=========================================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the project root directory"
    exit 1
fi

# Check if dependencies are installed
echo "Checking dependencies..."
if ! command -v cmake &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "⚠️  Dependencies not found. Running setup script..."
    if [ -x "./setup_dependencies.sh" ]; then
        ./setup_dependencies.sh --yes
    else
        echo "Error: setup_dependencies.sh not found or not executable"
        exit 1
    fi
fi

# Check for Qt5 if building GUI components
if [ "$BUILD_TARGET" != "daemon" ] && [ "$BUILD_TARGET" != "wallet" ]; then
    if ! pkg-config --exists Qt5Core 2>/dev/null; then
        echo "⚠️  Qt5 not found. Installing GUI dependencies..."
        if [ -x "./setup_dependencies.sh" ]; then
            ./setup_dependencies.sh --yes
        fi
    fi
    
    # Check for Qt Wayland support (required for GUI miner)
    if ! dpkg -l | grep -q "qtwayland5\|libqt5waylandclient5" 2>/dev/null; then
        echo "⚠️  Qt Wayland support not found. Installing Qt Wayland dependencies..."
        echo "This is required for the GUI miner to work properly."
        if command -v apt &> /dev/null; then
            sudo apt update && sudo apt install -y qtwayland5 libqt5waylandclient5 libqt5waylandcompositor5
        else
            echo "Please install qtwayland5, libqt5waylandclient5, and libqt5waylandcompositor5 packages"
        fi
    fi
fi

# Check and initialize submodules if needed
echo "Checking git submodules..."
MISSING_SUBMODULES=false

# Check for all required submodules
if [ ! -d "external/trezor-common" ] || [ ! -f "external/trezor-common/protob/messages.proto" ]; then
    echo "  ❌ trezor-common submodule missing or incomplete"
    MISSING_SUBMODULES=true
fi

if [ ! -d "external/randomx" ] || [ ! -f "external/randomx/CMakeLists.txt" ]; then
    echo "  ❌ randomx submodule missing or incomplete"
    MISSING_SUBMODULES=true
fi

if [ ! -d "external/miniupnp" ] || [ ! -f "external/miniupnp/miniupnpc/CMakeLists.txt" ]; then
    echo "  ❌ miniupnp submodule missing or incomplete"
    MISSING_SUBMODULES=true
fi

if [ ! -d "external/rapidjson" ] || [ ! -f "external/rapidjson/CMakeLists.txt" ]; then
    echo "  ❌ rapidjson submodule missing or incomplete"
    MISSING_SUBMODULES=true
fi

if [ "$MISSING_SUBMODULES" = true ]; then
    echo "Initializing git submodules..."
    echo "  Adding missing submodules..."
    
    # Add submodules if they don't exist in .gitmodules
    if ! git config --file .gitmodules --get-regexp "submodule.*trezor-common" >/dev/null 2>&1; then
        echo "    Adding trezor-common submodule..."
        git submodule add https://github.com/trezor/trezor-common.git external/trezor-common 2>/dev/null || true
    fi
    
    if ! git config --file .gitmodules --get-regexp "submodule.*randomx" >/dev/null 2>&1; then
        echo "    Adding randomx submodule..."
        git submodule add https://github.com/tevador/RandomX.git external/randomx 2>/dev/null || true
    fi
    
    if ! git config --file .gitmodules --get-regexp "submodule.*miniupnp" >/dev/null 2>&1; then
        echo "    Adding miniupnp submodule..."
        git submodule add https://github.com/miniupnp/miniupnp.git external/miniupnp 2>/dev/null || true
    fi
    
    if ! git config --file .gitmodules --get-regexp "submodule.*rapidjson" >/dev/null 2>&1; then
        echo "    Adding rapidjson submodule..."
        git submodule add https://github.com/Tencent/rapidjson.git external/rapidjson 2>/dev/null || true
    fi
    
    echo "  Updating all submodules..."
    git submodule update --init --recursive --force
    
    echo "  Applying patches..."
    if [ -f "patches/miniupnp_strdup_fix.patch" ]; then
        echo "    Applying miniupnp strdup fix patch..."
        cd external/miniupnp
        if git apply --check ../../patches/miniupnp_strdup_fix.patch 2>/dev/null; then
            git apply ../../patches/miniupnp_strdup_fix.patch
            echo "    ✅ miniupnp strdup fix applied successfully"
        else
            echo "    ⚠️  miniupnp strdup fix patch already applied or not applicable"
        fi
        cd ../..
    fi
    
    # Fallback: Direct file modification if patch fails
    echo "  Applying fallback fixes..."
    if [ -f "external/miniupnp/miniupnpc/CMakeLists.txt" ]; then
        if ! grep -q "_GNU_SOURCE" external/miniupnp/miniupnpc/CMakeLists.txt; then
            echo "    Applying miniupnp strdup fix (fallback method)..."
            sed -i 's/_BSD_SOURCE _DEFAULT_SOURCE/_BSD_SOURCE _DEFAULT_SOURCE _GNU_SOURCE/g' external/miniupnp/miniupnpc/CMakeLists.txt
            echo "    ✅ miniupnp strdup fix applied (fallback method)"
        else
            echo "    ✅ miniupnp strdup fix already applied"
        fi
    fi
    
    echo "  Verifying submodules..."
    if [ ! -d "external/trezor-common" ] || [ ! -f "external/trezor-common/protob/messages.proto" ]; then
        echo "    ❌ Failed to initialize trezor-common"
        exit 1
    fi
    if [ ! -d "external/randomx" ] || [ ! -f "external/randomx/CMakeLists.txt" ]; then
        echo "    ❌ Failed to initialize randomx"
        exit 1
    fi
    if [ ! -d "external/miniupnp" ] || [ ! -f "external/miniupnp/miniupnpc/CMakeLists.txt" ]; then
        echo "    ❌ Failed to initialize miniupnp"
        exit 1
    fi
    if [ ! -d "external/rapidjson" ] || [ ! -f "external/rapidjson/CMakeLists.txt" ]; then
        echo "    ❌ Failed to initialize rapidjson"
        exit 1
    fi
    
    echo "  ✅ All submodules initialized successfully"
else
    echo "  ✅ All submodules are present and complete"
fi

# Clean build directory if it exists (to avoid CMake cache issues)
if [ -d "build" ]; then
    echo "Cleaning existing build directory to avoid cache conflicts..."
    rm -rf build/
fi

# Apply critical fixes before CMake configuration
echo "Applying critical build fixes..."
if [ -f "external/miniupnp/miniupnpc/CMakeLists.txt" ]; then
    if ! grep -q "_GNU_SOURCE" external/miniupnp/miniupnpc/CMakeLists.txt; then
        echo "  Fixing miniupnp strdup compilation error..."
        # Try multiple patterns to ensure we catch the right line
        sed -i 's/_BSD_SOURCE _DEFAULT_SOURCE/_BSD_SOURCE _DEFAULT_SOURCE _GNU_SOURCE/g' external/miniupnp/miniupnpc/CMakeLists.txt
        sed -i 's/MINIUPNPC_GET_SRC_ADDR/MINIUPNPC_GET_SRC_ADDR\n    _GNU_SOURCE/g' external/miniupnp/miniupnpc/CMakeLists.txt
        # If still not found, add it manually
        if ! grep -q "_GNU_SOURCE" external/miniupnp/miniupnpc/CMakeLists.txt; then
            echo "    Adding _GNU_SOURCE manually..."
            sed -i '/MINIUPNPC_GET_SRC_ADDR/a\    _GNU_SOURCE' external/miniupnp/miniupnpc/CMakeLists.txt
        fi
        echo "  ✅ miniupnp strdup fix applied"
    else
        echo "  ✅ miniupnp strdup fix already applied"
    fi
else
    echo "  ⚠️  miniupnp CMakeLists.txt not found, skipping strdup fix"
fi

# Fix usleep compilation error for newer Ubuntu versions
if [ -f "external/miniupnp/miniupnpc/src/minihttptestserver.c" ]; then
    if ! grep -q "_XOPEN_SOURCE" external/miniupnp/miniupnpc/src/minihttptestserver.c; then
        echo "  Fixing usleep compilation error..."
        sed -i '1i#define _XOPEN_SOURCE 600' external/miniupnp/miniupnpc/src/minihttptestserver.c
        echo "  ✅ usleep fix applied"
    else
        echo "  ✅ usleep fix already applied"
    fi
else
    echo "  ⚠️  minihttptestserver.c not found, skipping usleep fix"
fi

# Setup QSF configuration directory and files
echo "Setting up QSF configuration..."
if [ -x "./setup_qsf_environment.sh" ]; then
    ./setup_qsf_environment.sh
else
    echo "⚠️  setup_qsf_environment.sh not found, creating basic config..."
    QSF_CONFIG_DIR="$HOME/.quantumsafefoundation"
    mkdir -p "$QSF_CONFIG_DIR"
    
    if [ ! -f "$QSF_CONFIG_DIR/qsf.local.conf" ]; then
        echo "Creating basic qsf.local.conf..."
        cat > "$QSF_CONFIG_DIR/qsf.local.conf" << 'EOF'
# QSF Local Daemon Configuration
rpc-bind-ip=127.0.0.1
rpc-bind-port=18071
p2p-bind-ip=0.0.0.0
p2p-bind-port=18070
zmq-rpc-bind-ip=0.0.0.0
zmq-rpc-bind-port=18072
zmq-pub=tcp://0.0.0.0:18073
log-level=1
max-concurrency=1
out-peers=8
in-peers=8
add-priority-node=seeds.qsfchain.com:18070
add-priority-node=seeds.qsfnetwork.co:18070
add-priority-node=seeds.qsfcoin.org:18070
add-priority-node=seeds.qsfcoin.com:18070
EOF
        echo "✅ Created basic $QSF_CONFIG_DIR/qsf.local.conf"
    fi
fi

# Parse command line arguments
BUILD_TYPE="Release"
BUILD_TARGET="all"

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --daemon-only)
            BUILD_TARGET="daemon"
            shift
            ;;
        --gui-miner-only)
            BUILD_TARGET="gui-miner"
            shift
            ;;
        --wallet-only)
            BUILD_TARGET="wallet"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug          Build in debug mode"
            echo "  --daemon-only    Build only the daemon"
            echo "  --gui-miner-only Build only the GUI miner"
            echo "  --wallet-only    Build only the wallet CLI"
            echo "  --help           Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Build everything in release mode"
            echo "  $0 --debug            # Build everything in debug mode"
            echo "  $0 --daemon-only      # Build daemon only"
            echo ""
            echo "Note: The build script automatically handles:"
            echo "  - Submodule initialization and verification"
            echo "  - Build directory cleanup (avoids cache conflicts)"
            echo "  - Dependency checking and installation"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Create build directory
BUILD_DIR="build"
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake (Build Type: $BUILD_TYPE)..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DMANUAL_SUBMODULES=1 \
    -DBUILD_QUANTUM_SAFE_MINER=ON \
    -DBUILD_GUI_DEPS=ON \
    -DQSF_QUANTUM_SAFE_ENABLED=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_FLAGS="-O2 -march=native" \
    -DCMAKE_C_FLAGS="-O2 -march=native"

# Build based on target
echo "Building QSF components..."
        case $BUILD_TARGET in
            "daemon")
                echo "Building daemon only..."
                make daemon -j1
                ;;
            "gui-miner")
                echo "Building GUI miner only..."
                make qsf-gui-miner -j1
                ;;
            "wallet")
                echo "Building wallet CLI only..."
                make simplewallet -j1
                ;;
            "all")
                echo "Building all components..."
                make -j1
                ;;
esac

echo "=========================================="
echo "Build completed successfully!"
echo "=========================================="
echo ""
echo "Build directory: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"
echo ""
echo "Executables available in $BUILD_DIR/bin/:"
echo "  • qsf                    - QSF daemon"
echo "  • qsf-gui-miner          - QSF GUI miner"
echo "  • qsf-wallet-cli         - QSF wallet CLI"
echo "  • qsf-wallet-rpc         - QSF wallet RPC"
echo ""
echo "Setting up memory-aware configurations..."
cd ..
if [ -x "./setup_memory_optimized_config.sh" ]; then
    ./setup_memory_optimized_config.sh
else
    echo "⚠️  Memory configuration script not found, using default config"
fi
echo ""
echo "To run components:"
echo "  ./start_qsf_daemon.sh              # Smart launcher (recommended)"
echo "  cd $BUILD_DIR/bin && ./qsf         # Start daemon with default config"
echo "  cd $BUILD_DIR/bin && ./qsf-gui-miner  # Start GUI miner"
echo "  cd $BUILD_DIR/bin && ./qsf-wallet-cli # Start wallet CLI"
echo ""
echo "Quantum-safe features are MANDATORY and always enabled."
echo "=========================================="
