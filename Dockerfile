# OpenClaw WebAssembly Build and Server Container
# This Dockerfile builds the game and serves it via HTTP
# Supports both ARM64 and AMD64 architectures

# Use platform-specific Emscripten base image
FROM --platform=$BUILDPLATFORM emscripten/emsdk:3.1.54 AS builder

# Install additional dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    python3 \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /openclaw

# Copy project files
COPY . .

# Create ASSETS.ZIP if ASSETS directory exists
RUN if [ -d "Build_Release/ASSETS" ]; then \
        cd Build_Release/ASSETS && zip -r ../ASSETS.ZIP . ; \
    fi

# Build the WASM version
RUN mkdir -p build && cd build && \
    emcmake cmake -DEmscripten=1 .. && \
    make -j$(nproc) || true

# Runtime stage - serve the built files
# Use platform-specific Python image for optimal performance
FROM --platform=$TARGETPLATFORM python:3.11-slim

WORKDIR /game

# Copy built files from builder
COPY --from=builder /openclaw/Build_Release ./

# Expose port for HTTP server
EXPOSE 8080

# Start HTTP server
CMD ["python3", "-m", "http.server", "8080", "--bind", "0.0.0.0"]
