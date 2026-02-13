#!/bin/bash

# OpenClaw Docker Quick Start Script
# Runs the WASM game in a Docker container via Colima

set -e

echo "=== OpenClaw Docker Runner ==="
echo ""

# Check if Colima is running
if ! colima status &> /dev/null; then
    echo "❌ Colima is not running!"
    echo "Start it with: colima start"
    exit 1
fi

echo "✅ Colima is running"
echo ""

# Detect architecture
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    IS_ARM=true
    echo "✅ ARM64 architecture detected (Apple Silicon or ARM)"
else
    IS_ARM=false
    echo "✅ x86_64 architecture detected"
fi
echo ""

# Function to display menu
show_menu() {
    echo "Choose an option:"
    echo ""
    echo "1) 🎮 Run with pre-built WASM files (fastest - no rebuild)"
    if [ "$IS_ARM" = true ]; then
        echo "2) 🔨 Build from source (multi-arch, ~15-20 minutes)"
        echo "3) ⚡ Build from source (ARM64-optimized, ~8-12 minutes, recommended)"
    else
        echo "2) 🔨 Build from source and run (takes ~10-15 minutes)"
    fi
    echo "4) 🧹 Stop and remove containers"
    echo "5) 📊 View container logs"
    echo "6) 🐚 Shell into container"
    echo "7) ❌ Exit"
    echo ""
}

# Check if required files exist
check_files() {
    if [ ! -f "Build_Release/CLAW.REZ" ]; then
        echo "⚠️  Warning: Build_Release/CLAW.REZ not found!"
    fi

    if [ ! -f "Build_Release/ASSETS.ZIP" ]; then
        echo "⚠️  Warning: Build_Release/ASSETS.ZIP not found!"
    fi

    if [ -f "Build_Release/openclaw.wasm" ]; then
        echo "✅ Pre-built WASM files found ($(du -h Build_Release/openclaw.wasm | cut -f1))"
    else
        echo "ℹ️  No pre-built WASM files found - will need to build"
    fi
}

# Option 1: Run pre-built
run_prebuilt() {
    echo "🎮 Starting OpenClaw with pre-built files..."
    echo ""

    docker-compose --profile prebuilt up -d openclaw-prebuilt

    echo ""
    echo "✅ Server started!"
    echo "🌐 Open in your browser:"
    echo "   http://localhost:8081/openclaw.html"
    echo ""
    echo "To stop: docker-compose down"
}

# Option 2: Build and run (multi-arch)
build_and_run() {
    echo "🔨 Building OpenClaw from source (multi-arch)..."
    echo "⏱️  This will take 15-20 minutes..."
    echo ""

    docker-compose build openclaw
    docker-compose up -d openclaw

    echo ""
    echo "✅ Build complete and server started!"
    echo "🌐 Open in your browser:"
    echo "   http://localhost:8080/openclaw.html"
    echo ""
    echo "To view build logs: docker-compose logs -f openclaw"
    echo "To stop: docker-compose down"
}

# Option 3: Build and run (ARM64-optimized)
build_and_run_arm64() {
    echo "⚡ Building OpenClaw from source (ARM64-optimized)..."
    echo "⏱️  This will take 8-12 minutes on Apple Silicon..."
    echo ""

    docker-compose --profile arm64 build openclaw-arm64
    docker-compose --profile arm64 up -d openclaw-arm64

    echo ""
    echo "✅ Build complete and server started!"
    echo "🌐 Open in your browser:"
    echo "   http://localhost:8082/openclaw.html"
    echo ""
    echo "💡 Note: ARM64 build is ~2x faster than multi-arch!"
    echo ""
    echo "To view build logs: docker-compose --profile arm64 logs -f openclaw-arm64"
    echo "To stop: docker-compose --profile arm64 down"
}

# Option 3: Stop containers
stop_containers() {
    echo "🧹 Stopping containers..."
    docker-compose down
    echo "✅ Containers stopped"
}

# Option 4: View logs
view_logs() {
    echo "📊 Viewing container logs (Ctrl+C to exit)..."
    echo ""
    docker-compose logs -f
}

# Option 5: Shell into container
shell_into_container() {
    echo "🐚 Opening shell in container..."
    echo ""
    if docker ps | grep -q openclaw-prebuilt; then
        docker exec -it openclaw-prebuilt /bin/bash
    elif docker ps | grep -q openclaw-wasm; then
        docker exec -it openclaw-wasm /bin/bash
    else
        echo "❌ No running OpenClaw container found!"
        echo "Start one first with option 1 or 2"
    fi
}

# Main script
check_files
echo ""

# If pre-built files exist, default to option 1
if [ -f "Build_Release/openclaw.wasm" ]; then
    echo "💡 Recommended: Use option 1 (pre-built files are available)"
fi

echo ""

while true; do
    show_menu
    read -p "Enter choice [1-6]: " choice
    echo ""

    case $choice in
        1)
            run_prebuilt
            break
            ;;
        2)
            build_and_run
            break
            ;;
        3)
            if [ "$IS_ARM" = true ]; then
                build_and_run_arm64
                break
            else
                stop_containers
            fi
            ;;
        4)
            stop_containers
            ;;
        5)
            view_logs
            ;;
        6)
            shell_into_container
            ;;
        7)
            echo "👋 Goodbye!"
            exit 0
            ;;
        *)
            if [ "$IS_ARM" = true ]; then
                echo "❌ Invalid choice. Please enter 1-7."
            else
                echo "❌ Invalid choice. Please enter 1-7."
            fi
            echo ""
            ;;
    esac
done
