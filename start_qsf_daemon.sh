#!/bin/bash

# QSF Smart Daemon Launcher
# Automatically selects the best configuration for your system

set -e

echo "=========================================="
echo "QSF Smart Daemon Launcher"
echo "=========================================="

# Network selection
if [ "$1" = "--testnet" ]; then
    NETWORK="testnet"
    CONFIG_FILE="qsf-testnet.conf"
    echo "Using TESTNET configuration"
elif [ "$1" = "--local" ]; then
    NETWORK="local"
    CONFIG_FILE="qsf-local-dev.conf"
    echo "Using LOCAL DEVELOPMENT configuration (offline mode)"
else
    NETWORK="mainnet"
    # Detect memory and select configuration
    TOTAL_MEM=$(free -m | awk 'NR==2{printf "%.0f", $2}')

    if [ "$TOTAL_MEM" -lt 2048 ]; then
        CONFIG_FILE="qsf-mainnet-lowmem.conf"
        echo "Using low-memory configuration (${TOTAL_MEM}MB RAM)"
    elif [ "$TOTAL_MEM" -lt 4096 ]; then
        CONFIG_FILE="qsf-mainnet-medium.conf"
        echo "Using medium-memory configuration (${TOTAL_MEM}MB RAM)"
    else
        CONFIG_FILE="qsf-mainnet-high.conf"
        echo "Using high-memory configuration (${TOTAL_MEM}MB RAM)"
    fi
fi

# Check if configuration exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: Configuration file $CONFIG_FILE not found"
    echo "Available configurations:"
    echo "  ./start_qsf_daemon.sh          # Mainnet (auto-detect memory)"
    echo "  ./start_qsf_daemon.sh --testnet # Testnet"
    echo "  ./start_qsf_daemon.sh --local   # Local development (offline)"
    exit 1
fi

echo "Starting QSF daemon with $CONFIG_FILE..."
echo ""

# Start the daemon
cd build/bin
./qsf --config-file=../../$CONFIG_FILE

