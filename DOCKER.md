# OpenClaw Docker Setup

Run OpenClaw WASM build inside Docker (via Colima) and access from your Mac browser.

## Quick Start

### Prerequisites
- Colima installed and running: `colima start`
- Docker CLI installed
- CLAW.REZ and ASSETS.ZIP in `Build_Release/`

### Option 1: Use Pre-Built Files (Fastest) ⚡

Since you already have `openclaw.wasm` built, just serve it:

```bash
./docker-run.sh
# Select option 1

# Or directly:
docker-compose --profile prebuilt up -d openclaw-prebuilt
```

**Open in browser:** http://localhost:8081/openclaw.html

### Option 2: Build from Source 🔨

Build the WASM files inside the container:

```bash
./docker-run.sh
# Select option 2

# Or directly:
docker-compose up -d --build openclaw
```

**Open in browser:** http://localhost:8080/openclaw.html

## Commands

### Start server with pre-built files
```bash
docker-compose --profile prebuilt up -d openclaw-prebuilt
```

### Build and run
```bash
docker-compose up -d --build openclaw
```

### View logs
```bash
docker-compose logs -f
```

### Stop containers
```bash
docker-compose down
```

### Shell into container
```bash
docker exec -it openclaw-prebuilt /bin/bash
# or
docker exec -it openclaw-wasm /bin/bash
```

## Architecture

```
┌─────────────────────────────────────────┐
│  Mac Browser (Safari/Chrome)            │
│  http://localhost:8080/openclaw.html    │
└─────────────────┬───────────────────────┘
                  │ Port 8080/8081
                  │
┌─────────────────▼───────────────────────┐
│  Colima (Docker Engine)                 │
│  ┌─────────────────────────────────┐   │
│  │  Docker Container               │   │
│  │  ┌───────────────────────────┐ │   │
│  │  │ Python HTTP Server        │ │   │
│  │  │ Serving: openclaw.wasm    │ │   │
│  │  │          openclaw.js      │ │   │
│  │  │          openclaw.html    │ │   │
│  │  │          openclaw.data    │ │   │
│  │  └───────────────────────────┘ │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

## Port Mapping

- **8080** → Build from source container
- **8081** → Pre-built files container

## Volumes

The `docker-compose.yml` mounts `./Build_Release` as read-only into the container, so:
- Changes to your local files are reflected immediately
- No need to rebuild container for asset changes
- Just refresh your browser

## Troubleshooting

### Colima not running
```bash
colima start
# or with more resources:
colima start --cpu 4 --memory 8
```

### Port already in use
```bash
# Check what's using the port
lsof -i :8080

# Use a different port
docker-compose --profile prebuilt up -d openclaw-prebuilt -p 9090:8080
```

### WASM files not loading
1. Check browser console (F12)
2. Verify files exist: `ls -lh Build_Release/*.wasm`
3. Check container logs: `docker-compose logs -f`

### WebGPU not working
- Use Chrome/Edge for best WebGPU support
- Check `chrome://gpu` to verify WebGPU is enabled
- May need to enable flags: `chrome://flags/#enable-unsafe-webgpu`

## Benefits of This Approach

✅ **No native build issues** - Linux container handles all compilation
✅ **Consistent environment** - Same build every time
✅ **Mac security bypassed** - Container runs the build, not macOS
✅ **Easy cleanup** - `docker-compose down` removes everything
✅ **Multiple versions** - Run different branches in different containers
✅ **Resource isolation** - Container has its own CPU/memory limits

## File Sizes

Expected sizes in `Build_Release/`:
- `openclaw.wasm`: ~46MB (compiled game code)
- `openclaw.data`: ~113MB (packed assets)
- `openclaw.js`: ~434KB (JavaScript wrapper)
- `CLAW.REZ`: ~113MB (original game assets)
- `ASSETS.ZIP`: ~466KB (custom assets)

## Next Steps

1. Run `./docker-run.sh` and choose option 1
2. Open http://localhost:8081/openclaw.html
3. Game should load with WebGPU graphics and Web Audio
4. Check browser console for any errors

Enjoy Captain Claw! 🐱‍👤
