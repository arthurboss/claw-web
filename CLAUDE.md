# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Overview

OpenClaw WASM is a browser-based fork of Captain Claw (1997), focused on optimizing the WebAssembly build. The original archive (`CLAW.REZ`, ~113MB) is **not** bundled: the user uploads it once and it's stored compressed in IndexedDB. Startup is fast (~48MB download, 2-3s) because level assets lazy-load on demand.

## Prerequisites

- **`CLAW.REZ`** — original game archive (~113MB, uploaded at runtime, not in repo).
- **`ASSETS.ZIP`** — custom assets, auto-generated from `Build_Release/ASSETS/`.
- Emscripten SDK (`emsdk/`, gitignored), CMake 4.3.2+, Node.js 18+ (Vite).
- Browsers: Chrome 105+ / Firefox 121+ / Safari 16.4+ / Edge 105+.

## Build

```bash
source ./emsdk/emsdk_env.sh
./build_wasm.sh          # regenerate ASSETS.ZIP, configure, compile, patch SDL2 shaders, relink
cd build && make -j$(nproc)   # C++-only incremental build
rm -rf build && ./build_wasm.sh   # clean build
```

> **SDL2 shader patch** (`patch_sdl2_shaders.sh`, run by `build_wasm.sh`) is required for WebGL: it disables `GL_OES_EGL_image_external` and swaps `samplerExternalOES` for `sampler2D`. A hand-rolled `cmake`/`make` build must run it after the first `make`, then clear `emsdk/upstream/emscripten/cache/build/sdl2` and rebuild.

## Running Locally

```bash
yarn dev                 # Vite at http://localhost:5173/
```

Vite gives hot reload and rewrites `/` → `/openclaw.html`. Serve over `localhost` (a **secure context**), which Keyboard Lock and Web Audio / AudioWorklet require; a plain-HTTP LAN IP breaks those APIs.

## Deployment (GitHub Pages)

Deploys are manual via `scripts/`. Both environments share the `gh-pages` branch: **production at root**, **staging under `/staging/`**. Neither script can clobber the other (prod excludes `staging/` from `--delete`; staging writes only `/staging/`). Both verify the WASM artifacts are fresh (guards the `ASM_CONSTS` crash from a stale loader+wasm pair) and self-clean their temp worktree.

- **Production:** <https://arthurboss.github.io/WASM-OpenClaw/> — `./scripts/deploy-prod.sh ["msg"]`
- **Staging:** <https://arthurboss.github.io/WASM-OpenClaw/staging/> — `./scripts/deploy-staging.sh ["msg"]`

**Default workflow:** deploy every new branch to **staging first** (safe — never touches prod), test on a real device, then merge and run `deploy-prod.sh` to promote. Skip only for deploy-only/non-visual changes.

**Env isolation** (prod + staging share one origin, so storage is scoped deliberately):

- **SW cache** named per scope (`openclaw::<scope>::<version>`) — a version bump in one env can't delete the other's cache; kill-switch teardown is scoped too.
- **`localStorage`** namespaced by scope: prod keeps bare keys (preserves existing saves), `/staging/` gets a `staging:` prefix. A `Storage.prototype` shim inlined in `<head>` does this — it **must** stay inline (runs before the save glue baked into `openclaw.js`), unlike the external `sw-register.js` / `env-marker.js`.
- **`IndexedDB`** (`CLAW.REZ` + assets) is intentionally **shared** — staging needs no re-upload.
- Gold **STAGING** badge marks non-production builds.

## Architecture

### Layout

```text
OpenClaw/Engine/       # Core C++ engine
  Actor/ Audio/ Events/ GameApp/ Physics/ Resource/ Scene/ UserInterface/ Process/ Logger/ Util/
  Graphics/WASM/       # WebGL renderer (Graphics/Generic/ = platform-agnostic abstraction)
libwap/ Box2D/ ThirdParty/     # WAP parser, physics, deps (TinyXML, FastDelegate, sigc++)
Build_Release/         # Output: openclaw.{wasm,js,data,html}, config.xml, *-bridge.js, ASSETS/, ASSETS.ZIP
scripts/ docs/ emsdk/(gitignored)
```

### Lazy-loading

Startup loads only menu/UI assets (~150ms). Level assets and level metadata (XML) load on first entry to each level; a resource cache (default 150 items, in `config.xml`) holds hot assets. Only visited levels consume memory.

### Assets & resources

- First run: no `CLAW.REZ` in IndexedDB → upload UI → compress → store (~62MB). Every load: retrieve → decompress (~200-500ms) → mount to FS. Compression auto-selects zstd → brotli → gzip via native `CompressionStream` (gzip in practice today), falling back to uncompressed on failure.
- `ASSETS.ZIP` is preloaded with the WASM (<1MB) and **overrides** `CLAW.REZ` files with matching paths.
- Resource paths are **case-sensitive, forward-slash**: `/STATES/MENU/*`, `/GAME/IMAGES/MENU/*`, `/GAME/FONTS/*`, `/LEVEL1..14/*`.
- Level metadata XMLs are gzip-compressed inside `ASSETS.ZIP`; `XmlLoader.cpp` decompresses transparently. Edit via `scripts/decompress_metadata.sh` → edit → `scripts/compress_metadata.sh`.

### C++/JS bridge

C++ calls JS via `EM_ASM` / `EM_ASM_INT` (strings via `UTF8ToString($0)`). Bridge files in `Build_Release/` are ES6 modules copied at build time, imported by `openclaw.html`: `asset-loader.js` / `asset-storage.js` (IndexedDB + compression), `resource-loader.js` (loading coordination), `graphics-bridge.js` / `texture-bridge.js` (WebGL + textures).

## Development Workflow

**When to rebuild:**

- **Full rebuild** (C++ source, `CMakeLists.txt`, deps, flags): `cd build && make -j$(nproc)`.
- **JS / HTML / CSS / root `config.xml`**: no rebuild, hard-refresh the browser.
- **XML or assets under `Build_Release/ASSETS/`**: rebuild `ASSETS.ZIP` **and force a relink** — the #1 silent gotcha. `ASSETS.ZIP` is embedded into `openclaw.data` only during the Emscripten link step, which CMake skips if nothing else changed:

  ```bash
  ./scripts/decompress_metadata.sh          # if editing metadata XMLs
  # ...edit XMLs under Build_Release/ASSETS/ ...
  ./scripts/compress_metadata.sh
  cd Build_Release && rm -f ASSETS.ZIP && (cd ASSETS && zip -q -r ../ASSETS.ZIP .) && cd ..
  touch Build_Release/ASSETS.ZIP            # forces CMake to relink
  cd build && cmake .. && make -j$(nproc)
  ```

**Never commit build artifacts:** `build/`, `Build_Release/openclaw.{wasm,js,data}`, `Build_Release/ASSETS.ZIP`, `emsdk/`.

**Testing:** no automated tests. Build → `yarn dev` → check browser console for JS errors and C++ `LOG` output → play affected levels. In-game console (`~`): `reload levelmetadata`.

## Key Files

- **Lazy loading:** `BaseGameApp.cpp` — startup skips eager metadata (:173), `VPreload` startup assets (:186-196), `LoadSingleLevelMetadata()` (:1356), `GetLevelMetadata()` lazy (:1545).
- **Assets:** `Resource/ResourceMgr.cpp`, `Resource/ResourceCache.cpp`.
- **Graphics:** `Graphics/WASM/PureWebGLRenderer.cpp`, `Graphics/WASM/TextureManager.cpp`.
- **Audio:** `Audio/WebAudioAPI.cpp`, `Audio/WASM/AudioWorkletSystem.cpp`, `Resource/Loaders/WavLoader.cpp` (memory-safe buffer copies); `HumanView.cpp` sound (:641), MIDI disabled for Emscripten (:562).
- **Metadata:** `Resource/Loaders/XmlLoader.cpp`.

## Build Configuration

```bash
emcmake cmake -DEmscripten=1 ..                     # WASM mode
emcmake cmake -DEmscripten=1 -DExtern_Config=0 ..   # embed config.xml in WASM
```

Emscripten flags live in `CMakeLists.txt` (SDL2 + image/ttf/gfx/mixer, `ASYNCIFY`, 256MB memory with growth, WebGL2, `USE_ZLIB`, `FETCH`). Preloaded: `ASSETS.ZIP`, `console02.tga`, `clacon.ttf`. `CLAW.REZ` is explicitly **not** preloaded.

## Known Limitations

- MIDI audio not supported on WASM (WAV/OGG fine).
- Firefox: ALT opens the window menu (browser bug) — use fullscreen.
- Death/teleport fade effects can have rendering quirks.

## Docs & References

- `docs/` — player (`GAMEPAD.md`, `TROUBLESHOOTING.md`), developer (`BUILDING.md`, `ARCHITECTURE.md`, `SAVE_SYSTEM.md`, `HAPTIC_FEEDBACK.md`, `SCREEN_RENDERING.md`, `GENERIC_GRAPHICS.md`).
- Original OpenClaw: <https://github.com/pjasicek/OpenClaw> · Emscripten: <https://emscripten.org/docs/> · SDL2: <https://wiki.libsdl.org/>
