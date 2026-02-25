# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> **Note:** Local environment settings and personal preferences are in `CLAUDE.local.md` (gitignored)

## Project Overview

OpenClaw WASM is a browser-based version of Captain Claw (1997), a classic platformer game. This is a WASM-only fork that focuses on optimizing the WebAssembly implementation for modern browsers. The game uses assets from the original game archive (CLAW.REZ) which users upload once and store in IndexedDB.

**Key Features:**

- Fast loading: Only 48MB download, 2-3 second startup
- Lazy-loading architecture: Assets load on-demand as you play
- One-time setup: Upload CLAW.REZ once, play forever
- Full game: All 14 levels with original graphics and audio

## Prerequisites

### Required Files

- `CLAW.REZ` - Original game archive (112MB, user uploads at runtime)
- `ASSETS.ZIP` - Custom assets archive (auto-generated from `Build_Release/ASSETS/`)

### Build Tools

- Emscripten SDK (in `emsdk/` directory, gitignored)
- CMake 4.10+
- Python 3 (for local web server)
- Caddy server (optional, for HTTP/3 support)

### Browser Requirements

- Chrome 105+ / Firefox 121+ / Safari 16.4+ / Edge 105+
- ~160MB free storage (for cached game files in IndexedDB)

## Build Commands

### Quick Build (Recommended)

```bash
source ./emsdk/emsdk_env.sh
./build_wasm.sh
```

**What it does:**

1. Regenerates ASSETS.ZIP from Build_Release/ASSETS/
2. Configures CMake for Emscripten
3. Compiles C++ to WebAssembly
4. Patches SDL2 shaders for WebGL compatibility
5. Rebuilds with patched shaders
6. Shows build summary

**Build time:** 5-10 minutes (first build), 1-2 minutes (incremental)

### Manual Build (Advanced)

```bash
# 1. Activate Emscripten environment
source ./emsdk/emsdk_env.sh

# 2. Configure with CMake
mkdir build
cd build
emcmake cmake -DEmscripten=1 ..

# 3. First build (downloads SDL2)
make -j$(nproc)

# 4. Patch SDL2 shaders for WebGL compatibility
cd ..
./patch_sdl2_shaders.sh

# 5. Clear SDL2 cache and rebuild
rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2
cd build
make -j$(nproc)
```

**Important:** SDL2 shader patching is required to disable `GL_OES_EGL_image_external` and replace `samplerExternalOES` with `sampler2D` for WebGL compatibility.

## Running the Game Locally

### Option 1: HTTP/3 Server (Recommended - Faster)

```bash
./scripts/start_http3_server.sh
# Opens at https://localhost:8080/openclaw.html
```

**Benefits:**

- HTTP/3 (QUIC protocol) for 30-50% faster loading
- Brotli/Zstd/Gzip compression
- Automatic HTTPS
- HTTP/2 fallback

### Option 2: Python Server (Simple)

```bash
cd Build_Release
python3 -m http.server 8080
# Opens at http://localhost:8080/openclaw.html
```

## Code Architecture

### Directory Structure

```
OpenClaw/
├── OpenClaw/Engine/          # Core C++ engine implementation
│   ├── Actor/                # Game actor system (player, enemies, items, projectiles)
│   ├── Audio/                # Sound and music management (Web Audio API for WASM)
│   ├── Events/               # Event system for game logic communication
│   ├── GameApp/              # Main loop, game logic, save system, metadata loader
│   ├── Graphics/             # 2D rendering and image handling
│   │   ├── WASM/             # WebGL-specific renderer
│   │   └── Generic/          # Platform-agnostic graphics abstraction
│   ├── Logger/               # Logging system
│   ├── Physics/              # Box2D physics integration
│   ├── Process/              # Process management system
│   ├── Resource/             # Resource loading/caching (REZ/ZIP archives)
│   ├── Scene/                # Scene management and level loading
│   ├── UserInterface/        # UI components and menus
│   └── Util/                 # Utilities (XML, converters, profilers)
├── libwap/                   # WAP (game format) file parsing library
├── Box2D/                    # Physics engine
├── ThirdParty/               # Dependencies (TinyXML, FastDelegate, sigc++)
├── Build_Release/            # Output directory
│   ├── ASSETS/               # Custom assets (source)
│   ├── ASSETS.ZIP            # Packaged assets (generated)
│   ├── openclaw.wasm         # Compiled game (~40MB)
│   ├── openclaw.js           # Emscripten runtime (~8MB)
│   ├── openclaw.data         # Preloaded assets
│   ├── openclaw.html         # Game entry point
│   ├── config.xml            # Game configuration
│   ├── asset-loader.js       # IndexedDB bridge
│   ├── asset-storage.js      # Asset storage abstraction
│   ├── resource-loader.js    # Resource loading coordination
│   ├── graphics-bridge.js    # WebGL graphics bridge
│   └── texture-bridge.js     # Texture management bridge
├── docs/                     # Documentation
├── scripts/                  # Build and server scripts
└── emsdk/                    # Emscripten SDK (gitignored)
```

### Key Architectural Patterns

**Lazy-Loading System:**

- **Startup:** Only loads menu/UI assets (~150ms)
- **Level Load:** Assets load on-demand when entering levels
- **Metadata:** Level metadata (XML configs) load lazily on first level entry
- **Caching:** Resource cache stores frequently accessed assets (default: 150 items)
- **Result:** Fast startup regardless of game size, only visited levels consume memory

**IndexedDB Asset Storage with Compression:**

**Key concept:** CLAW.REZ (113MB) is NOT bundled in WASM build. It's compressed and stored in IndexedDB.

**Flow:**

1. User opens game → Check IndexedDB for CLAW.REZ
2. If missing → Show upload UI (one-time)
3. User uploads → Auto-detect best compression (zstd → brotli → gzip) → Store in IndexedDB (~62MB with gzip)
4. All subsequent loads → Retrieve from IndexedDB → Decompress (~200-500ms) → Mount to FS

**Compression Implementation:**

- Multi-algorithm support: zstd (best) → brotli → gzip (fallback)
- Uses browser-native `CompressionStream` / `DecompressionStream` APIs
- Currently uses gzip (as of Chrome 145, zstd/brotli not yet in CompressionStream API)
- Future-proof: Will automatically use zstd/brotli when browsers add support
- Compression: One-time during upload, adds 2-5 seconds
- Decompression: Every game load, adds 200-500ms (~10-20% startup time)
- Storage savings: 45% reduction with gzip (113MB → 62MB)
- Automatic fallback to uncompressed if compression/decompression fails

**Files:**
- `asset-loader.js` - Multi-algorithm compression/decompression and IndexedDB bridge
- `asset-storage.js` - Stores compression algorithm metadata

**Browser Compatibility:** Chrome 80+, Firefox 113+, Safari 16.4+, Edge 80+ (gzip supported on all)

**Resource Management:**

- CLAW.REZ stored compressed in browser's IndexedDB (one-time upload, 45-70MB compressed)
- Decompressed to 113MB in memory at game load
- Resources extracted on-demand when requested (not all at startup)
- ASSETS.ZIP preloaded with WASM (bundled, <1MB)
- Path-based organization: `/LEVEL1/*`, `/LEVEL2/*`, etc.
- Custom assets override CLAW.REZ files with same paths

**Resource Path Conventions:**

**Important:** All resource paths are case-sensitive and use forward slashes:

- Menu assets: `/STATES/MENU/*`, `/GAME/IMAGES/MENU/*`
- Fonts: `/GAME/FONTS/*`
- Level assets: `/LEVEL1/*`, `/LEVEL2/*`, etc. (14 levels total)
- Custom assets in ASSETS.ZIP override CLAW.REZ by matching paths exactly

**Game Loop:**

- Main loop in `GameApp/MainLoop.cpp`
- Process-based system for game logic updates
- Event-driven communication between components

**Physics:**

- Box2D integration for platformer physics
- Separate physics world management

**Audio System:**

- Web Audio API for audio playback in WASM builds
- All audio loaded from CLAW.REZ via resource system (no HTTP fetches)
- Memory-safe buffer copying to prevent use-after-free bugs
- SDL_Mixer for native builds (if any)
- Configurable sound/music volumes and channels

**Actor System:**

- Component-based actor architecture
- Actor definitions loaded from XML
- Various actor types: player, enemies, items, projectiles, triggers, effects

**C++/JavaScript Bridge Pattern:**

**Calling JavaScript from C++:**
Use `EM_ASM` for inline JavaScript execution:

```cpp
#include <emscripten.h>

EM_ASM({
    console.log('Hello from C++');
    window.someFunction();
});
```

**Passing data between C++ and JavaScript:**

```cpp
// C++ → JS (passing values)
int result = EM_ASM_INT({
    return window.someJSFunction($0, $1);
}, arg1, arg2);

// C++ → JS (strings)
EM_ASM({
    console.log('Message: ' + UTF8ToString($0));
}, message.c_str());
```

**Key bridge modules:**

- `Build_Release/asset-loader.js` - IndexedDB operations for CLAW.REZ
- `Build_Release/asset-storage.js` - Asset storage abstraction
- `Build_Release/graphics-bridge.js` - WebGL graphics bridge
- `Build_Release/texture-bridge.js` - Texture management
- `Build_Release/resource-loader.js` - Resource loading coordination

**Pattern:** Bridge files are ES6 modules copied during build (see CMakeLists.txt:69-82). They're imported by the main game HTML and provide JavaScript APIs that C++ code can call via `EM_ASM`.

## Configuration

Game configuration is in `Build_Release/config.xml`:

- Display settings (resolution, fullscreen, vsync, scale)
- Audio settings (frequency, channels, volumes)
- Asset paths (REZ archive, custom ZIP)
- Resource cache size (default: 150, increase for more RAM)
- Console appearance and behavior
- Global game options (physics parameters, timing)

## Development Workflow

### When to Rebuild

**Full rebuild required:**

- Modified C++ source files (`*.cpp`, `*.h`)
- Changed CMakeLists.txt
- Updated C++ dependencies (Box2D, libwap)
- Changed build flags or Emscripten settings

**Partial rebuild (ASSETS.ZIP and relink):**

- Modified XML files in `Build_Release/ASSETS/` (e.g., `MENU.xml`, level XMLs)
- Changed custom assets in `Build_Release/ASSETS/`
- **CRITICAL:** ASSETS.ZIP is embedded into `openclaw.data` during the Emscripten linking phase. Simply rebuilding ASSETS.ZIP is NOT enough - you must force a relink!
- **Complete workflow (required for XML/asset changes):**

  ```bash
  # Step 1: Decompress metadata for editing (if needed)
  ./scripts/decompress_metadata.sh

  # Step 2: Edit XMLs
  # ... make your changes ...

  # Step 3: Compress metadata before rebuild
  ./scripts/compress_metadata.sh

  # Step 4: Rebuild ASSETS.ZIP
  cd Build_Release
  rm -f ASSETS.ZIP
  cd ASSETS
  zip -q -r ../ASSETS.ZIP .
  cd ../..

  # Step 2: Force CMake to relink (picks up updated ASSETS.ZIP)
  touch Build_Release/ASSETS.ZIP
  cd build
  cmake ..
  make -j$(nproc)
  ```

- **After rebuild:** Hard refresh browser (Ctrl+Shift+R or Cmd+Shift+R) to clear cached WASM files
- **Why this is needed:** Emscripten embeds ASSETS.ZIP at link time using `--preload-file`. The linking step only runs when CMake detects changes to dependencies. Touching ASSETS.ZIP forces CMake to detect it as modified and trigger a relink.

**No rebuild needed (just refresh browser):**

- Modified JavaScript files (`*.js` in Build_Release/)
- Modified HTML (`openclaw.html`)
- Modified CSS styles
- Changed `config.xml` (root level, not in ASSETS/)
- Updated documentation

### Incremental Builds

After first build, only modified files recompile:

```bash
source ./emsdk/emsdk_env.sh
cd build
make -j$(nproc)
```

### Clean Build

To start fresh:

```bash
rm -rf build
./build_wasm.sh
```

### Build Artifacts (Gitignored)

Never commit these generated files:

- `build/` - CMake build directory
- `Build_Release/openclaw.{wasm,js,data}` - Compiled outputs
- `emsdk/` - Emscripten SDK (large, user-installed)
- `Build_Release/ASSETS.ZIP` - Auto-generated from ASSETS/

## Testing

This project does not have automated tests. Verify changes by:

1. **Build:** Run `./build_wasm.sh` to compile changes
2. **Run:** Start local server (`./scripts/start_http3_server.sh` or Python server)
3. **Browser Console:** Check for JavaScript errors and warnings (F12)
4. **Manual Testing:** Play through affected levels/features
5. **Log Output:** Watch browser console for C++ logs (uses `LOG` macro)

## Key Implementation Files

### Lazy Loading

- `OpenClaw/Engine/GameApp/BaseGameApp.cpp:173` - Startup (skips eager metadata load)
- `OpenClaw/Engine/GameApp/BaseGameApp.cpp:1356` - `LoadSingleLevelMetadata()`
- `OpenClaw/Engine/GameApp/BaseGameApp.cpp:1545` - `GetLevelMetadata()` (with lazy load)
- `OpenClaw/Engine/GameApp/BaseGameApp.cpp:186-196` - VPreload calls (startup assets)

### Asset Management

- `OpenClaw/Engine/Resource/ResourceMgr.cpp` - Resource manager implementation
- `OpenClaw/Engine/Resource/ResourceCache.cpp` - Resource caching logic
- `Build_Release/asset-loader.js` - IndexedDB bridge for CLAW.REZ
- `Build_Release/asset-storage.js` - Asset storage abstraction layer
- `Build_Release/resource-loader.js` - Resource loading coordination

### Graphics System

- `OpenClaw/Engine/Graphics/WASM/PureWebGLRenderer.cpp` - WebGL renderer
- `OpenClaw/Engine/Graphics/WASM/TextureManager.cpp` - Texture loading/management
- `Build_Release/graphics-bridge.js` - Graphics system bridge
- `Build_Release/texture-bridge.js` - Texture management bridge

### Audio System

- `OpenClaw/Engine/Audio/WebAudioAPI.cpp` - Web Audio API integration
- `OpenClaw/Engine/Audio/WASM/AudioWorkletSystem.cpp` - Audio playback from CLAW.REZ data
- `OpenClaw/Engine/Resource/Loaders/WavLoader.cpp` - WAV loading with memory-safe buffer copying
- `OpenClaw/Engine/UserInterface/HumanView.cpp:641` - Sound playback delegation
- `OpenClaw/Engine/UserInterface/HumanView.cpp:562` - MIDI handling (disabled for Emscripten)

### Metadata Compression

- Level metadata XMLs are gzip-compressed to reduce ASSETS.ZIP size by ~79%
- `OpenClaw/Engine/Resource/Loaders/XmlLoader.cpp` - Automatic gzip decompression
- `scripts/compress_metadata.sh` - Compress XMLs before build
- `scripts/decompress_metadata.sh` - Decompress XMLs for editing
- See `docs/METADATA_COMPRESSION.md` for details

**Compression results:**

- Original: 28,895 bytes (13 XML files)
- Compressed: 5,955 bytes (79.4% smaller)
- Runtime: ~0.1ms decompression per file (negligible impact)

## Build Configuration

### CMake Options

```bash
emcmake cmake -DEmscripten=1 ..                    # WebAssembly mode
emcmake cmake -DEmscripten=1 -DExtern_Config=0 ..  # Embed config.xml in WASM
```

### Emscripten Flags (in CMakeLists.txt)

Key flags set in CMakeLists.txt line 98:

```cmake
-s WASM=1                    # WebAssembly output
-s USE_SDL=2                 # SDL2 support
-s USE_SDL_IMAGE=2           # SDL2_image
-s USE_SDL_TTF=2             # SDL2_ttf
-s USE_SDL_GFX=2             # SDL2_gfx
-s USE_SDL_MIXER=2           # SDL2_mixer
-s ASYNCIFY=1                # Enable async operations
-s TOTAL_MEMORY=268435456    # 256MB memory allocation
-s ALLOW_MEMORY_GROWTH=1     # Dynamic memory growth
-s FULL_ES3=1                # WebGL2 support
-s USE_WEBGL2=1              # WebGL2 API
-s MIN_WEBGL_VERSION=2       # Minimum WebGL version
-s MAX_WEBGL_VERSION=2       # Maximum WebGL version
-s USE_ZLIB=1                # Enable zlib for gzip decompression
-s FETCH                     # Enable fetch API for async loading
```

**Preloaded files:**

- `ASSETS.ZIP` - Custom assets bundle
- `console02.tga` - Console background
- `clacon.ttf` - Console font

**Note:** CLAW.REZ is explicitly NOT preloaded (lazy-loaded from IndexedDB).

## Testing and Debugging

### Browser Console Logs

**Startup (should see):**

```
INFO: Loading critical assets (menu/UI)...
INFO: Critical assets loaded. Level assets will load on-demand.
INFO: Actor prototypes loaded successfully.
```

**First level entry (should see):**

```
INFO: Loading assets for LEVEL2...
INFO: Level assets loaded for LEVEL2
INFO: Requesting metadata for level 2
INFO: Lazy-loaded metadata for level 2
```

**Second level entry (should NOT see metadata messages):**

```
INFO: Loading assets for LEVEL2...
INFO: Level assets loaded for LEVEL2
```

### Console Commands

In-game console (press ~ key):

- `reload levelmetadata` - Reloads all metadata files (development feature)

### Common Issues

**Black screen:**

1. Open browser console (F12)
2. Check for WebGL errors
3. Verify SDL2 shaders were patched correctly
4. Check if CLAW.REZ upload completed successfully

**Asset loading errors:**

1. Verify CLAW.REZ is in IndexedDB (check Application tab in DevTools)
2. Clear IndexedDB and re-upload CLAW.REZ
3. Check resource paths are case-sensitive and use forward slashes

## Known Limitations

**Browser-Specific:**

- Firefox ALT key opens window menu (browser bug) - use fullscreen or disable in about:config
- Microsoft Edge may have WAV playback issues in older versions

**WASM Platform:**

- MIDI audio not yet supported (WAV/OGG work fine). Might add Web MIDI support
- Death/teleport fade effects may have rendering quirks

## Recent Changes

- **Audio system fix**: Fixed use-after-free bug causing game sounds to fail with "Invalid WAV header" errors
  - All sounds now load directly from CLAW.REZ via data parameter
  - Eliminated duplicate audio files and HTTP fetch overhead
  - Proper memory management with buffer copying in WavLoader
- Lazy-loading architecture for faster startup
- HTTP/3 server support with Caddy (30-50% faster loading)
- Brotli/Zstd compression for optimal transfer size
- SDL2 shader patches for WebGL compatibility
- IndexedDB storage for CLAW.REZ (one-time upload)
- Volume defaults optimized for browser playback

## Documentation

See `docs/` directory for detailed information:

- **SETUP.md** - Detailed setup instructions for players
- **BUILDING.md** - Complete build guide for developers
- **ARCHITECTURE.md** - Technical architecture and lazy-loading design
- **TROUBLESHOOTING.md** - Common issues and solutions
- **HTTP3_MIGRATION.md** - HTTP/3 server setup and benefits

## References

- Original OpenClaw: <https://github.com/pjasicek/OpenClaw>
- Emscripten Documentation: <https://emscripten.org/docs/>
- SDL2 Documentation: <https://wiki.libsdl.org/>
