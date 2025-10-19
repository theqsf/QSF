#!/bin/bash

# QSF Environment Setup Script
# This script sets up the complete QSF environment including dependencies and configuration

set -e

echo "=========================================="
echo "QSF Environment Setup"
echo "=========================================="

# Get the user's home directory
USER_HOME="$HOME"
QSF_CONFIG_DIR="$USER_HOME/.quantumsafefoundation"

echo "Setting up QSF environment for user: $(whoami)"
echo "Configuration directory: $QSF_CONFIG_DIR"

# Create QSF configuration directory
echo "Creating QSF configuration directory..."
mkdir -p "$QSF_CONFIG_DIR"

# Create qsf.local.conf
echo "Setting up qsf.local.conf..."
cat > "$QSF_CONFIG_DIR/qsf.local.conf" << EOF
# QSF Local Daemon Configuration
# This file is used by the GUI miner for local daemon startup

# Quantum-Safe Features (always enabled)
quantum-safe=1
dual-quantum-enforcement=1
quantum-hybrid=1
enforce-quantum-safe=1
randomx-quantum-integration=1
xmss-tree-height=10
sphincs-level=5

# RPC Configuration
rpc-bind-ip=127.0.0.1
rpc-bind-port=18071
restricted-rpc=1

# P2P Configuration
p2p-bind-ip=0.0.0.0
p2p-bind-port=18070
public-node=1

# ZMQ Configuration for mining
zmq-rpc-bind-ip=0.0.0.0
zmq-rpc-bind-port=18072
zmq-pub=tcp://0.0.0.0:18073

# Data and logging
data-dir=$USER_HOME/.quantumsafefoundation
log-level=1
log-file=$USER_HOME/.quantumsafefoundation/qsf-local.log
max-log-file-size=104857600
max-log-files=5

# Performance settings
max-concurrency=1
out-peers=8
in-peers=8
block-sync-size=2048
db-sync-mode=fast:async:250000000
prune-blockchain=1

# Network settings
igd=disabled
hide-my-port=0

# Seed nodes
add-priority-node=seeds.qsfchain.com:18070
add-priority-node=seeds.qsfnetwork.co:18070
add-priority-node=seeds.qsfcoin.org:18070
add-priority-node=seeds.qsfcoin.com:18070
EOF

echo "✅ Created $QSF_CONFIG_DIR/qsf.local.conf"

# Create a basic qsf.conf if it doesn't exist
if [ ! -f "$QSF_CONFIG_DIR/qsf.conf" ]; then
    echo "Setting up qsf.conf..."
    cat > "$QSF_CONFIG_DIR/qsf.conf" << EOF
# QSF Daemon Configuration (Auto-generated)
# This file was automatically created by the setup script

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
block-sync-size=2048
db-sync-mode=fast:async:250000000
prune-blockchain=1
igd=disabled
hide-my-port=0
# Seed Nodes
add-priority-node=seeds.qsfchain.com:18070
add-priority-node=seeds.qsfnetwork.co:18070
add-priority-node=seeds.qsfcoin.org:18070
add-priority-node=seeds.qsfcoin.com:18070
EOF
    echo "✅ Created $QSF_CONFIG_DIR/qsf.conf"
else
    echo "✅ qsf.conf already exists"
fi

# Check and install Qt Wayland dependencies if needed
echo "Checking Qt Wayland dependencies..."
if ! dpkg -l | grep -q "qtwayland5\|libqt5waylandclient5" 2>/dev/null; then
    echo "⚠️  Qt Wayland support not found. Installing Qt Wayland dependencies..."
    echo "This is required for the GUI miner to work properly."
    if command -v apt &> /dev/null; then
        sudo apt update && sudo apt install -y qtwayland5 libqt5waylandclient5 libqt5waylandcompositor5
        echo "✅ Qt Wayland dependencies installed"
    else
        echo "❌ Please install qtwayland5, libqt5waylandclient5, and libqt5waylandcompositor5 packages"
        echo "   For Ubuntu/Debian: sudo apt install qtwayland5 libqt5waylandclient5 libqt5waylandcompositor5"
        echo "   For other distributions, please install the equivalent Qt Wayland packages"
    fi
else
    echo "✅ Qt Wayland dependencies are already installed"
fi

# Set proper permissions
echo "Setting up permissions..."
chmod 755 "$QSF_CONFIG_DIR"
chmod 644 "$QSF_CONFIG_DIR"/*.conf 2>/dev/null || true

echo ""
echo "=========================================="
echo "QSF Environment Setup Complete!"
echo "=========================================="
echo ""
echo "Configuration files created:"
echo "  • $QSF_CONFIG_DIR/qsf.local.conf (for GUI miner)"
echo "  • $QSF_CONFIG_DIR/qsf.conf (for daemon)"
echo ""
echo "You can now:"
echo "  • Run ./build.sh to build QSF"
echo "  • Use ./start_qsf_daemon.sh to start the daemon"
echo "  • Run the GUI miner from build/bin/qsf-gui-miner"
echo ""
echo "Quantum-safe features are MANDATORY and always enabled."
echo "=========================================="
