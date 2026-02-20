# OpenClaw WASM

> WebAssembly port of Captain Claw (1997) that runs in modern browsers

Browser-based reimplementation of the classic platformer using Emscripten and WebGL. Based on [OpenClaw](https://github.com/pjasicek/OpenClaw) by pjasicek.

## ⚠️ Asset Requirements

**You will need CLAW.REZ** from the original Captain Claw (1997) game. The game will prompt you to upload this file on first run.

**Legal:** You must own the original game to use its assets legally. See [SETUP.md](SETUP.md) for details.

**Note:** This is a WASM-only fork. For desktop builds, visit the [original repository](https://github.com/pjasicek/OpenClaw).

## Features

- **Lazy Loading Architecture:**
  - Initial download ~48MB (vs 160MB traditional build)
  - Only menu/UI assets load at startup
  - Level assets load on-demand when entering levels
  - Level metadata (XML) loads on first visit, then cached
- **IndexedDB Storage:** Upload CLAW.REZ once, cached forever in browser
- **WebGL Rendering:** SDL2 with hardware acceleration
- **Responsive Design:** Automatic 4:3 aspect ratio scaling
- **HTTP/3 Support:** Optional fast loading with QUIC protocol

See [ARCHITECTURE.md](ARCHITECTURE.md) for technical details about the lazy loading system.

## Quick Start

### Running the Game

**Option 1: HTTP/3 Server (Recommended)**

```bash
./scripts/start_http3_server.sh
# Open https://localhost:8080/openclaw.html
```

**Option 2: Python Server**

```bash
cd Build_Release
python3 -m http.server 8080
# Open http://localhost:8080/openclaw.html
```

**Important:** The server must run from the `Build_Release` directory where `openclaw.html` and the WASM files are located.

### First Run

1. Browser will show "First Time Setup" screen
2. Select your CLAW.REZ file from the original game
3. File uploads and stores in browser (one-time process)
4. Game starts automatically

Subsequent visits load instantly from browser cache.

## Building from Source

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- Python 3 and CMake
- Linux or WSL (Windows not directly supported)

**Install Emscripten:**

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build

```bash
source ./emsdk/emsdk_env.sh
./build_wasm.sh
```

The build script automatically:

- Regenerates ASSETS.ZIP from source
- Patches SDL2 shaders for WebGL compatibility
- Creates WASM binaries in Build_Release/

**Note:** CLAW.REZ is NOT bundled in the build. Users upload their own at runtime.

### When to Rebuild

**Rebuild required:**

- Modified C++ source files (*.cpp,*.h)
- Changed CMakeLists.txt or build configuration
- Updated C++ dependencies (Box2D, libwap)

**Rebuild NOT required (just refresh browser):**

- Modified JavaScript files (asset-loader.js, graphics-bridge.js, etc.)
- Modified HTML (openclaw.html)
- Modified CSS styles
- Updated documentation

## Architecture

### Tech Stack

- **Graphics:** SDL2 + WebGL
- **Audio:** SDL_Mixer + Web Audio API
- **Physics:** Box2D
- **Build:** Emscripten (C++ to WebAssembly)

### File Structure

```markdown
Build_Release/
├── openclaw.wasm          # Game engine (47 MB)
├── openclaw.data          # Critical assets (600 KB) - was 113 MB!
├── openclaw.js            # JavaScript glue code
├── openclaw.html          # Main game page
├── asset-storage.js       # IndexedDB wrapper
├── asset-loader.js        # Emscripten FS integration
├── resource-loader.js     # Progress tracking
├── graphics-bridge.js     # WebGL bridge
└── texture-bridge.js      # Texture management
```

### Lazy Loading Flow

1. User visits page → Show upload UI if CLAW.REZ not cached
2. User uploads CLAW.REZ → Store in IndexedDB (113 MB)
3. Mount to Emscripten virtual filesystem
4. Load critical assets (menu/UI)
5. Game starts
6. Level-specific assets load on-demand

## Configuration

Edit `Build_Release/config.xml` for game settings:

```xml
<GlobalOptions>
  <ShowFps>true</ShowFps>           <!-- FPS counter -->
  <GameLogicFps>60</GameLogicFps>   <!-- Game update rate -->
</GlobalOptions>
```

## Browser Compatibility

- ✅ Chrome 105+
- ✅ Firefox 121+
- ✅ Safari 16.4+
- ✅ Edge 105+

Requires: WebAssembly, IndexedDB, File API

## Troubleshooting

Having issues? See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for:

- How to clear IndexedDB storage and re-upload CLAW.REZ
- Common error messages and solutions
- Storage quota issues
- Browser-specific problems

## Contributing

This is a WASM-focused fork. Pull requests welcome for:

- Browser compatibility improvements
- Performance optimizations
- Bug fixes
- Documentation

## License

GNU GPL v3 - See LICENSE file

Original game assets (CLAW.REZ) remain copyright Monolith Productions.

## Credits

- **Original Game:** Monolith Productions (1997)
- **OpenClaw Engine:** [pjasicek](https://github.com/pjasicek/OpenClaw)
- **WASM Fork:** Arthur Boss
