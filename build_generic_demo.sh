#!/bin/bash

# Build script for GenericGraphicsModule demo
# This script demonstrates how to build a simple application using the GenericGraphicsModule

echo "Building GenericGraphicsModule Demo..."

# Create build directory
mkdir -p build_demo
cd build_demo

# Configure with Emscripten
source ../emsdk/emsdk_env.sh

emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXECUTABLE_SUFFIX=.html \
    -DEmscripten=ON \
    -DCMAKE_CXX_FLAGS="-std=c++11 -O2"

# Build the demo
make -j4

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📁 Output files:"
    ls -la *.html *.js *.wasm 2>/dev/null || echo "   (No output files found)"
    echo ""
    echo "🚀 To run the demo:"
    echo "   1. Start a local web server in the build_demo directory"
    echo "   2. Open the generated HTML file in your browser"
    echo "   3. Check the browser console for demo output"
    echo ""
    echo "💡 Example:"
    echo "   cd build_demo"
    echo "   python3 -m http.server 8000"
    echo "   # Then open http://localhost:8000/generic_graphics_demo.html"
else
    echo "❌ Build failed!"
    exit 1
fi
