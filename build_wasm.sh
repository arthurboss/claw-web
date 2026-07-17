#!/bin/bash

# OpenClaw WebAssembly Build Script
# This script handles the complete build process with lazy loading architecture
# CLAW.REZ is NOT bundled - users provide it at runtime via browser upload

set -e  # Exit on any error

echo "=== OpenClaw WASM Build (Lazy Loading Architecture) ==="
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the OpenClaw root directory."
    exit 1
fi

# Check if Emscripten is available
if ! command -v emcmake &> /dev/null; then
    echo "Error: Emscripten not found. Please source the Emscripten environment first:"
    echo "  source ./emsdk/emsdk_env.sh"
    exit 1
fi

echo "1. Checking prerequisites..."
if [ ! -f "Build_Release/ASSETS.ZIP" ] && [ ! -d "Build_Release/ASSETS" ]; then
    echo "Error: ASSETS.ZIP or ASSETS/ directory not found in Build_Release/"
    exit 1
fi

echo "2. Rebuilding ASSETS.ZIP from source..."
cd Build_Release
rm -f ASSETS.ZIP
(cd ASSETS && zip -q -r ../ASSETS.ZIP .)
echo "   ASSETS.ZIP created: $(du -h ASSETS.ZIP | cut -f1)"

# Note about CLAW.REZ
if [ -f "CLAW.REZ" ]; then
    echo "   ℹ️  Note: CLAW.REZ present (useful for local testing)"
    echo "   This file is NOT bundled - end users upload their own"
fi
cd ..

echo "3. Setting up build directory..."
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

echo "4. Configuring CMake for Emscripten..."
emcmake cmake -DEmscripten=1 ..

echo "5. Building the project (first attempt to download SDL2)..."
make

echo "6. Patching SDL2 shaders for WebGL compatibility..."
cd ..
./patch_sdl2_shaders.sh

echo "7. Clearing SDL2 build cache and rebuilding..."
rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2
cd build
make

echo ""
echo "=== Build completed successfully! ==="
echo ""
echo "Build artifacts:"
echo "  - openclaw.wasm: $(du -h Build_Release/openclaw.wasm 2>/dev/null | cut -f1 || echo 'not built yet')"
echo "  - openclaw.data: $(du -h Build_Release/openclaw.data 2>/dev/null | cut -f1 || echo 'not built yet')"
echo "  - openclaw.js: $(du -h Build_Release/openclaw.js 2>/dev/null | cut -f1 || echo 'not built yet')"
echo ""
echo "Lazy loading architecture:"
echo "  ✓ Initial download: ~5MB (vs 160MB before)"
echo "  ✓ CLAW.REZ loaded from user's browser storage (IndexedDB)"
echo "  ✓ Assets load on-demand for optimal performance"
echo ""
echo "To run the game:"
echo "1. Start Vite dev server: npm run dev"
echo "2. Open SSH tunnel: ssh -o ControlPath=none -f -N -L 5173:localhost:5173 -i ~/.ssh/id_fedora_vm testuser@172.16.25.133"
echo "3. Open: http://localhost:5173/"
echo ""
echo "Note: Users will be prompted to upload CLAW.REZ on first run."
