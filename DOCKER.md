# OpenClaw Docker Setup

Simple HTTP server to serve pre-built WASM files on macOS.

## Overview

**Build**: Use Linux VM (Fedora ARM64) - native, fast (2-5 minutes)
**Serve**: Use Docker on macOS - simple HTTP server

## Quick Start

### 1. Build WASM in Linux VM
```bash
# Inside your Fedora VM
cd /path/to/OpenClaw
./build_wasm.sh
```

Files will sync to macOS via VMware shared folders.

### 2. Serve on macOS
```bash
# On macOS
docker-compose up -d
```

### 3. Open in Browser
http://localhost:8081/openclaw.html

## Commands

### Start server
```bash
docker-compose up -d
```

### Stop server
```bash
docker-compose down
```

### View logs
```bash
docker-compose logs -f
```

### Restart after rebuilding WASM
```bash
docker-compose restart
```

## How It Works

```
┌─────────────────────────────────────────┐
│  Mac Browser (Chrome/Safari)            │
│  http://localhost:8081/openclaw.html    │
└─────────────────┬───────────────────────┘
                  │ Port 8081
                  │
┌─────────────────▼───────────────────────┐
│  Docker Container (Python HTTP Server) │
│  Serving: Build_Release/ (read-only)   │
│  - openclaw.wasm                        │
│  - openclaw.js                          │
│  - openclaw.html                        │
│  - openclaw.data                        │
└─────────────────────────────────────────┘
```

## Workflow

1. **Edit code** on macOS (VSCode, any editor)
2. **Build WASM** in Fedora VM: `./build_wasm.sh` (~2-5 minutes)
3. **Files auto-sync** to macOS via VMware shared folders
4. **Refresh browser** - Docker serves the new files automatically
5. **Test and iterate**

## Troubleshooting

### Port already in use
```bash
lsof -i :8081
# Change port in docker-compose.yml if needed
```

### Files not updating
```bash
# Restart the container to reload
docker-compose restart
```

### Container not starting
```bash
# Check Colima is running
colima status

# Start Colima if needed
colima start
```

## Benefits

✅ **No build complexity** - Just serves files
✅ **Fast builds** - Native ARM64 in Linux VM (2-5 min)
✅ **Simple workflow** - Edit, build, refresh
✅ **Consistent serving** - Same HTTP environment every time
✅ **Easy cleanup** - `docker-compose down` removes everything

## Next Steps

See `VM_BUILD_SETUP.md` for Linux VM configuration.
