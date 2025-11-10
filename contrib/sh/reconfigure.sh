#!/bin/bash
# Reconfigure script to apply the ARM64 SSE4.2 fix

echo "Cleaning old configuration..."
rm -f config.status config.log

echo "Running configure..."
./configure "$@"

echo ""
echo "Checking SSE4.2 detection result..."
if grep -q "ENABLE_HWCRC32_TRUE.*#" config.status 2>/dev/null; then
    echo "✓ SSE4.2 is DISABLED (correct for ARM64)"
elif grep -q "ENABLE_HWCRC32_FALSE.*#" config.status 2>/dev/null; then
    echo "✗ SSE4.2 is ENABLED (incorrect for ARM64)"
    echo "  This should not happen on Apple Silicon!"
else
    echo "? Could not determine SSE4.2 status"
fi

echo ""
echo "You can now run: gmake"
