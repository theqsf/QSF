#!/bin/bash

# QSF Quantum-Safe Coin - Complete Setup Script
# This script handles all dependencies, submodules, and build preparation

set -e

echo "=========================================="
echo "QSF Quantum-Safe Coin - Complete Setup"
echo "=========================================="

# Parse options
INSTALL_GUI_DEPS=true  # Default to true for smoother experience
ASSUME_YES=false
for arg in "$@"; do
    case "$arg" in
        --with-gui)
            INSTALL_GUI_DEPS=true
            ;;
        --no-gui)
            INSTALL_GUI_DEPS=false
            ;;
        --yes|-y)
            ASSUME_YES=true
            ;;
    esac
done

# Check available memory
TOTAL_MEM=$(free -m | awk 'NR==2{printf "%.0f", $2}')
echo "System Memory: ${TOTAL_MEM}MB"

if [ "$TOTAL_MEM" -lt 2048 ]; then
    echo "⚠️  Warning: Low memory system detected (${TOTAL_MEM}MB)"
    echo "   Consider using ./build_server.sh for memory-optimized builds"
    echo ""
    if [ "$ASSUME_YES" = false ]; then
        # Offer to create swap file
        read -p "Create 2GB swap file to help with compilation? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Creating 2GB swap file..."
            sudo fallocate -l 2G /swapfile
            sudo chmod 600 /swapfile
            sudo mkswap /swapfile
            sudo swapon /swapfile
            echo "Swap file created and activated"
            echo ""
        fi
    fi
fi

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the project root directory"
    exit 1
fi

echo ""
echo "This script will:"
echo "1. Install system dependencies (including GUI dependencies)"
echo "2. Initialize git submodules"
echo "3. Set up build environment"
echo "4. Verify everything is ready"
echo ""
echo "Note: GUI dependencies (Qt5) are included by default for full functionality."
echo "Use --no-gui to skip GUI dependencies if you only need the daemon."
echo ""

# Ask for confirmation (skip when --yes)
if [ "$ASSUME_YES" = false ]; then
    read -p "Do you want to proceed with complete setup? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Setup cancelled."
        exit 0
    fi
fi

echo ""
echo "Starting complete setup..."

# 1. Install system dependencies
echo "1. Installing system dependencies..."
echo "   This may take a few minutes..."

# Detect OS and install dependencies
if command -v apt-get &> /dev/null; then
    # Ubuntu/Debian
    echo "   Detected Ubuntu/Debian system"
    sudo apt update
    sudo apt install -y \
        build-essential \
        cmake \
        pkg-config \
        libssl-dev \
        libzmq3-dev \
        libunbound-dev \
        libsodium-dev \
        libunwind8-dev \
        liblzma-dev \
        libreadline6-dev \
        libexpat1-dev \
        libpgm-dev \
        libhidapi-dev \
        libusb-1.0-0-dev \
        libprotobuf-dev \
        protobuf-compiler \
        libudev-dev \
        libboost-chrono-dev \
        libboost-date-time-dev \
        libboost-filesystem-dev \
        libboost-locale-dev \
        libboost-program-options-dev \
        libboost-regex-dev \
        libboost-serialization-dev \
        libboost-system-dev \
        libboost-thread-dev \
        python3 \
        ccache \
        doxygen \
        graphviz \
        git \
        curl

    if [ "$INSTALL_GUI_DEPS" = true ]; then
        echo "   Installing Qt5 GUI dependencies..."
        sudo apt install -y \
            qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5svg5-dev qttools5-dev-tools
    fi

elif command -v yum &> /dev/null; then
    # CentOS/RHEL/Fedora
    echo "   Detected CentOS/RHEL/Fedora system"
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y \
        cmake \
        pkg-config \
        openssl-devel \
        zeromq-devel \
        unbound-devel \
        libsodium-devel \
        libunwind-devel \
        xz-devel \
        readline-devel \
        expat-devel \
        protobuf-devel \
        protobuf-compiler \
        boost-devel \
        python3 \
        ccache \
        doxygen \
        graphviz \
        git \
        curl

    if [ "$INSTALL_GUI_DEPS" = true ]; then
        echo "   Installing Qt5 GUI dependencies..."
        sudo yum install -y qt5-qtbase-devel qt5-qttools-devel qt5-qtsvg-devel
    fi

elif command -v pacman &> /dev/null; then
    # Arch Linux
    echo "   Detected Arch Linux system"
    sudo pacman -S --needed \
        base-devel \
        cmake \
        pkg-config \
        openssl \
        zeromq \
        unbound \
        libsodium \
        libunwind \
        xz \
        readline \
        expat \
        protobuf \
        boost \
        python \
        ccache \
        doxygen \
        graphviz \
        git \
        curl

    if [ "$INSTALL_GUI_DEPS" = true ]; then
        echo "   Installing Qt5 GUI dependencies..."
        sudo pacman -S --needed qt5-base qt5-tools qt5-svg
    fi

else
    echo "   Warning: Could not detect package manager"
    echo "   Please install dependencies manually:"
    echo "   - build-essential/cmake/pkg-config"
    echo "   - libssl-dev/libzmq3-dev/libunbound-dev"
    echo "   - libsodium-dev/libboost-*-dev"
    echo "   - qttools5-dev-tools (for GUI)"
    echo "   - git/curl"
fi

echo "   ✓ System dependencies installed"

# 2. Initialize git submodules
echo ""
echo "2. Initializing git submodules..."

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "   Error: Not in a git repository"
    exit 1
fi

# Initialize submodules
echo "   Initializing submodules..."
git submodule update --init --recursive --force

# Add trezor-common if it doesn't exist
if [ ! -d "external/trezor-common" ]; then
    echo "   Adding trezor-common submodule..."
    git submodule add https://github.com/trezor/trezor-common.git external/trezor-common 2>/dev/null || true
    git submodule update --init --force
fi

# Verify critical submodules are properly initialized
echo "   Verifying submodules..."
if [ ! -d "external/randomx" ] || [ ! -f "external/randomx/CMakeLists.txt" ]; then
    echo "   Re-initializing randomx submodule..."
    git submodule update --init --force external/randomx
fi
if [ ! -d "external/miniupnp/miniupnpc" ]; then
    echo "   Re-initializing miniupnp submodule..."
    git submodule update --init --force external/miniupnp
fi
if [ ! -d "external/trezor-common" ]; then
    echo "   Re-initializing trezor-common submodule..."
    git submodule update --init --force external/trezor-common
fi

# Final update to ensure everything is in sync
echo "   Final submodule update..."
git submodule update --init --force

echo "   ✓ Git submodules initialized"

# 3. Set up build environment
echo ""
echo "3. Setting up build environment..."

# Create build directory
mkdir -p build

# Check if we have all required files
echo "   Checking required files..."
required_files=(
    "CMakeLists.txt"
    "src/cryptonote_config.h"
    "src/p2p/net_node.h"
    "build.sh"
    "build_gui_miner.sh"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ $file"
    else
        echo "   ✗ Missing: $file"
        exit 1
    fi
done

echo "   ✓ Build environment ready"

# 4. Verify everything is ready
echo ""
echo "4. Verifying setup..."

# Check git status
echo "   Git status:"
git status --porcelain | head -5
if [ $? -eq 0 ]; then
    echo "   ✓ Git repository OK"
else
    echo "   ⚠ Git repository issues detected"
fi

# Check submodules
echo "   Submodule status:"
git submodule status | head -5
if [ $? -eq 0 ]; then
    echo "   ✓ Submodules OK"
else
    echo "   ⚠ Submodule issues detected"
fi

# Check dependencies
echo "   Checking key dependencies:"
deps=("cmake" "g++" "pkg-config" "git")
for dep in "${deps[@]}"; do
    if command -v "$dep" &> /dev/null; then
        echo "   ✓ $dep"
    else
        echo "   ✗ Missing: $dep"
    fi
done

echo ""
echo "=========================================="
echo "Setup completed successfully!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Build the project:"
echo "   ./build.sh"
echo ""
echo "2. Or build GUI miner only:"
echo "   ./build_gui_miner.sh"
echo ""
echo "3. Run the daemon:"
echo "   cd build/bin && ./qsf"
echo ""
echo "4. Run the GUI miner:"
echo "   cd build/bin && ./qsf-gui-miner"
echo ""
echo "Note: If you encounter any issues, run this script again."
echo "=========================================="
