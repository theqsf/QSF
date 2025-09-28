#!/bin/bash

# Debug script for strdup fix
echo "=========================================="
echo "Debug: strdup Fix Status"
echo "=========================================="

echo "Current directory: $(pwd)"
echo ""

echo "Checking miniupnp CMakeLists.txt:"
if [ -f "external/miniupnp/miniupnpc/CMakeLists.txt" ]; then
    echo "  ✅ File exists"
    echo "  File size: $(wc -l < external/miniupnp/miniupnpc/CMakeLists.txt) lines"
    echo ""
    echo "Checking for _GNU_SOURCE:"
    if grep -q "_GNU_SOURCE" external/miniupnp/miniupnpc/CMakeLists.txt; then
        echo "  ✅ _GNU_SOURCE found"
        echo "  Lines containing _GNU_SOURCE:"
        grep -n "_GNU_SOURCE" external/miniupnp/miniupnpc/CMakeLists.txt
    else
        echo "  ❌ _GNU_SOURCE NOT found"
        echo ""
        echo "Checking compile definitions section:"
        grep -A 5 -B 5 "MINIUPNPC_GET_SRC_ADDR" external/miniupnp/miniupnpc/CMakeLists.txt
    fi
else
    echo "  ❌ File does not exist"
fi

echo ""
echo "Checking submodule status:"
if [ -d "external/miniupnp" ]; then
    echo "  ✅ miniupnp directory exists"
    cd external/miniupnp
    echo "  Git status:"
    git status --porcelain
    echo "  Current commit:"
    git log --oneline -1
    cd ../..
else
    echo "  ❌ miniupnp directory does not exist"
fi

echo ""
echo "=========================================="
