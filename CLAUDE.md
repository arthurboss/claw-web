# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## CRITICAL: Repository Policy

**⚠️ NEVER PUSH CHANGES TO THE REMOTE REPOSITORY! ⚠️**

This repository is maintained locally only. You may:

- Stage changes with `git add`
- Make branches locally

You must NEVER:

- Run `git commit` without explicit user approval
- Run `git push` or `git push origin`
- Push to any remote repository
- Create or update remote branches

**Git Commit Policy:**

- ALWAYS wait for user approval before creating commits
- Present changes for review and testing first
- Only commit after user explicitly requests it
- If the user requests to push changes, politely remind them of this policy and suggest alternatives like creating patch files or reviewing changes locally.

## Project Overview

OpenClaw is a C++ reimplementation of Captain Claw (1997), a classic platformer game. This fork focuses on fixing, updating, and modernizing the WebAssembly (WASM) implementation. The game uses assets from the original game archive (CLAW.REZ) and requires SDL2 libraries for graphics, input, audio, and Box2D for physics.

## Prerequisites

### Required Files

- `CLAW.REZ` - Original game archive (must be placed in `Build_Release/`)
- `ASSETS.ZIP` - Custom assets archive created from `Build_Release/ASSETS/` directory

### Platform-Specific Requirements

**Linux (Ubuntu/Debian):**

```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev
sudo apt install timidity freepats  # For MIDI music playback
```

**WebAssembly:**

- Emscripten SDK (included in `emsdk/` directory)
- Python 3 (for local web server)
- Caddy server (optional, for HTTP/3 support)

## Build Commands

### Native Build (Linux/macOS)

**Quick build and run:**

```bash
./build_and_run.sh
```

**Manual build:**

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)

# Create ASSETS.ZIP
cd ../Build_Release
rm -f ASSETS.ZIP
(cd ASSETS && zip -r ../ASSETS.ZIP .)

# Run the game
./openclaw
```

### WebAssembly Build

**Quick build (recommended):**

```bash
source ./emsdk/emsdk_env.sh
./build_wasm.sh
```

**Manual build:**

```bash
# 1. Activate Emscripten environment
source ./emsdk/emsdk_env.sh

# 2. Configure with CMake
mkdir build
cd build
emcmake cmake -DEmscripten=1 ..

# 3. First build (downloads SDL2)
make

# 4. Patch SDL2 shaders for WebGL compatibility
cd ..
./patch_sdl2_shaders.sh

# 5. Clear SDL2 cache and rebuild
rm -rf ./emsdk/upstream/emscripten/cache/build/sdl2
cd build
make
```

**Important WebAssembly Build Notes:**

- The build process requires patching SDL2 shaders (`patch_sdl2_shaders.sh`) to fix WebGL compatibility issues
- SDL2 shader patch disables `GL_OES_EGL_image_external` extension and replaces `samplerExternalOES` with `sampler2D`
- After patching, SDL2 build cache must be cleared for changes to take effect

### Running WebAssembly Build

**Option 1: HTTP/3 Server (recommended, faster loading):**

```bash
./scripts/start_http3_server.sh
# Opens at https://localhost:8080/openclaw.html
```

Features: HTTP/3 (QUIC), HTTP/2 fallback, Brotli/Zstd/Gzip compression, automatic HTTPS

**Option 2: Python HTTP/1.1 Server (legacy):**

```bash
python3 -m http.server 8080
# Opens at http://localhost:8080/Build_Release/openclaw.html
```

## Code Architecture

### Directory Structure

- `OpenClaw/Engine/` - Core engine implementation
  - `Actor/` - Game actor system (player, enemies, items, projectiles)
  - `Audio/` - Sound and music management
  - `Events/` - Event system for game logic communication
  - `GameApp/` - Main application loop, game logic, command handler, save system
  - `Graphics2D/` - 2D rendering and image handling
  - `Logger/` - Logging system
  - `Physics/` - Box2D physics integration
  - `Process/` - Process management system
  - `Resource/` - Resource loading and caching (REZ/ZIP archives)
  - `Scene/` - Scene management and level loading
  - `UserInterface/` - UI components and menus
  - `Util/` - Utility classes (XML parsing, converters, string utils, profilers)

- `libwap/` - WAP (game format) file parsing library
- `Box2D/` - Physics engine
- `ThirdParty/` - Third-party dependencies (Tinyxml, FastDelegate, sigc++)
- `Build_Release/` - Output directory for executables and game assets
- `emsdk/` - Emscripten SDK (gitignored, large external dependency)

### Key Architectural Patterns

**Resource Management:**

- Resources are loaded from `CLAW.REZ` (original game archive) and `ASSETS.ZIP` (custom content)
- Resource caching with configurable cache size (default 150, see `config.xml`)
- XML-driven data approach for game configuration

**Game Loop:**

- Main loop in `GameApp/MainLoop.cpp`
- Process-based system for game logic updates
- Event-driven communication between components

**Physics:**

- Box2D integration for platformer physics
- Separate physics world management

**Audio System:**

- SDL_Mixer for audio playback
- MIDI support via MidiProc (Windows) or timidity (Linux)
- Configurable sound/music volumes and channels

**Actor System:**

- Component-based actor architecture
- Actor definitions loaded from XML
- Various actor types: player, enemies, items, projectiles, triggers, effects

## Configuration

Game configuration is controlled via `Build_Release/config.xml`:

- Display settings (resolution, fullscreen, vsync, scale)
- Audio settings (frequency, channels, volumes)
- Asset paths (REZ archive, custom ZIP, resource cache size)
- Console appearance and behavior
- Global game options (physics parameters, timing)

## Platform-Specific Notes

### WebAssembly Limitations

- MIDI audio is disabled (unsupported in browsers)
- WAV file format may not work in all browsers (Microsoft Edge issues)
- Death/teleport fade effects are broken (Emscripten buffer issue)
- Firefox ALT key opens window menu (browser bug) - use fullscreen or disable in about:config
- Some SDL_Mixer functions not implemented (search for `TODO: [EMSCRIPTEN]`)

### Build Flags

**CMake Options:**

- `-DEmscripten=1` - Build for WebAssembly
- `-DExtern_Config=0` - Embed config.xml in WASM (default: external file)

**Emscripten Flags (in CMakeLists.txt):**

- `WASM=1` - WebAssembly output
- `USE_SDL=2` - SDL2 support
- `USE_SDL_IMAGE=2`, `USE_SDL_TTF=2`, `USE_SDL_GFX=2`, `USE_SDL_MIXER=2` - SDL extension libraries
- `ASYNCIFY=1` - Enable async operations
- `TOTAL_MEMORY=268435456` - 256MB memory allocation
- `FULL_ES3=1`, `USE_WEBGL2=1` - WebGL2 support

## Development Workflow

1. **Making Code Changes:** Edit source files in `OpenClaw/Engine/`
2. **Testing Native:** Use `./build_and_run.sh` for quick iteration
3. **Testing WASM:** Run `./build_wasm.sh` after sourcing Emscripten environment
4. **Debugging:** Check `Build_Release/config.xml` for console settings and startup commands

## Recent Changes

- HTTP/3 server support with Caddy for 30-50% faster loading (Brotli/Zstd compression)
- SDL2 shader patches for WebGL compatibility
- Volume defaults changed on game startup
- WASM audio fixes
