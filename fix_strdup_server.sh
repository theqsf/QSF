#!/bin/bash

# Comprehensive strdup fix for server
echo "=========================================="
echo "Applying comprehensive strdup fix..."
echo "=========================================="

cd external/miniupnp

# Restore original file first
echo "Restoring original CMakeLists.txt..."
git checkout -- miniupnpc/CMakeLists.txt

# Apply the correct fix
echo "Applying strdup fix..."
sed -i 's/_BSD_SOURCE _DEFAULT_SOURCE/_BSD_SOURCE _DEFAULT_SOURCE _GNU_SOURCE/g' miniupnpc/CMakeLists.txt

# Also add _GNU_SOURCE directly to the upnp-listdevices-static target
echo "Adding _GNU_SOURCE to upnp-listdevices-static target..."
sed -i '/add_executable (upnp-listdevices-static/a\  target_compile_definitions(upnp-listdevices-static PRIVATE _GNU_SOURCE)' miniupnpc/CMakeLists.txt

# Verify the changes
echo "Verifying changes..."
echo "1. Check _GNU_SOURCE in compile definitions:"
grep -A 3 -B 3 "_GNU_SOURCE" miniupnpc/CMakeLists.txt

echo ""
echo "2. Check upnp-listdevices-static target:"
grep -A 5 "upnp-listdevices-static" miniupnpc/CMakeLists.txt

cd ../..

echo "=========================================="
echo "Fix applied! Now try: ./build.sh"
echo "=========================================="
