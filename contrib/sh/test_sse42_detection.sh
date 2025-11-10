#!/bin/bash
# Test script to verify SSE4.2 detection on different architectures

echo "Testing SSE4.2 detection..."
echo "Current architecture: $(uname -m)"

# Run configure and capture the SSE4.2 detection output
./configure 2>&1 | grep -A1 "assembler crc32 support"

echo ""
echo "Checking if ENABLE_HWCRC32 is set in config.status..."
if [ -f config.status ]; then
    grep "ENABLE_HWCRC32" config.status | head -5
fi
