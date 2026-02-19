#!/bin/bash

# OpenClaw WebAssembly Build Script
# This script handles the complete build process including SDL2 shader patching

set -e  # Exit on any error

echo "=== OpenClaw WebAssembly Build Script ==="
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

echo "1. Rebuilding ASSETS.ZIP from source..."
cd Build_Release
rm -f ASSETS.ZIP
(cd ASSETS && zip -r ../ASSETS.ZIP .)
echo "   ASSETS.ZIP created: $(du -h ASSETS.ZIP | cut -f1)"
cd ..

echo "2. Setting up build directory..."
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

echo "3. Configuring CMake for Emscripten..."
emcmake cmake -DEmscripten=1 ..

echo "4. Building the project (first attempt to download SDL2)..."
make

echo "5. Patching SDL2 shaders for WebGL compatibility..."
cd ..
./patch_sdl2_shaders.sh

echo "6. Clearing SDL2 build cache and rebuilding..."
rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2
cd build
make

echo ""
echo "=== Build completed successfully! ==="
echo ""
echo "To run the game:"
echo "1. Start a web server: python3 -m http.server 8080"
echo "2. Open: http://localhost:8080/Build_Release/openclaw.html"
echo ""
echo "Note: Make sure you have CLAW.REZ and ASSETS.ZIP in the Build_Release directory."
