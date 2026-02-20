# OpenClaw Asset Architecture Improvement Plan

## Context

This is a WASM-focused fork of OpenClaw (Captain Claw 1997 reimplementation). The current architecture has several issues:

**Current Problems:**
1. **Massive initial download**: 160MB (openclaw.wasm 47MB + openclaw.data 113MB) must download before game starts
2. **Mixed desktop/WASM code**: Repository contains Windows-specific code (MidiProc, ClawLauncher, .exe/.dll files) that's unused in WASM builds
3. **Unclear setup**: Users don't know they need CLAW.REZ or how to obtain it legally
4. **No lazy loading**: All 113MB of CLAW.REZ assets load upfront, wasted for users who only play Level 1
5. **Build artifacts in repo**: 23 Windows binary files (.exe, .dll, .lib) unnecessarily tracked

**User's Goals:**
- Implement modern lazy loading using WASM and browser APIs (IndexedDB, File API)
- Make setup easier and clearer for users
- Optimize for 2026 web standards (progressive loading, compression)
- Focus solely on WASM build (this is a WASM-only fork)

**Why This Change:**
The current monolithic asset bundling made sense for desktop builds, but WASM + modern browsers enable a much better user experience: fast initial load, progressive asset streaming, and local caching. This aligns with web best practices in 2026.

---

## Implementation Plan

### Phase 1: Repository Cleanup (WASM-Only Focus)

**Goal:** Remove all Windows/desktop-specific code and build artifacts to clarify this is a WASM-only fork.

**Files to Delete:**
```
MidiProc/                           # Windows-only MIDI processor (entire directory)
ClawLauncher/                       # Windows-only C# launcher (entire directory)
Build_Release/*.exe                 # OpenClaw.exe, ClawLauncher.exe, MidiProc.exe
Build_Release/*.dll                 # All SDL2 and Windows DLLs (15 files)
Build_Release/*.lib                 # All .lib files (8 files)
Build_Release/*.pdb                 # Debug symbol files
Build_Release/msvcr120d.dll        # Visual Studio debug runtime
OpenClaw/Engine/Audio/midiproc_c.c # Windows MIDI RPC client stub
```

**Directories to Clean:**
```
build/                   # CMake build artifacts (regenerable)
build_demo/              # Demo build artifacts (regenerable)
cmake-build-debug/       # Debug build artifacts (regenerable)
Release/                 # Old release artifacts
```

**Update .gitignore:**
Add patterns to prevent future desktop artifacts:
```gitignore
# Desktop build artifacts (WASM-only fork)
*.exe
*.dll
*.lib
*.pdb
MidiProc/
ClawLauncher/
```

**Update CMakeLists.txt:**
- Remove MidiProc subdirectory addition (lines 24-26)
- Remove Windows-specific linking logic
- Keep Emscripten/WASM paths only
- Comment out or remove desktop graphics backends (WebGL/WebGPU with SDL)

**Critical Files:**
- `CMakeLists.txt` - Build configuration
- `.gitignore` - Ignore patterns
- `README.md` - Update to clarify WASM-only

---

### Phase 2: IndexedDB Asset Management System

**Goal:** Implement a system where users provide CLAW.REZ once, it's stored in IndexedDB, and assets are loaded lazily on-demand.

#### A. Create Asset Upload UI

**File:** `Build_Release/openclaw.html`

Add before game canvas:
```html
<div id="assetUpload" style="display: none;">
  <h2>First Time Setup</h2>
  <p>OpenClaw requires CLAW.REZ from the original game.</p>
  <p><strong>Legal Notice:</strong> You must own Captain Claw (1997) to use this file.</p>

  <div id="uploadArea">
    <input type="file" id="clawRezFile" accept=".REZ" />
    <button onclick="uploadClawRez()">Upload CLAW.REZ</button>
  </div>

  <div id="uploadProgress" style="display: none;">
    <progress id="uploadProgressBar" max="100" value="0"></progress>
    <span id="uploadStatus">Storing assets...</span>
  </div>
</div>
```

#### B. IndexedDB Storage Layer

**File:** `Build_Release/asset-storage.js` (new file)

Create IndexedDB wrapper for asset management:
```javascript
class AssetStorage {
  constructor() {
    this.dbName = 'OpenClawAssets';
    this.storeName = 'files';
    this.db = null;
  }

  async init() {
    // Open IndexedDB connection
    // Create object store for asset blobs
  }

  async storeFile(name, blob, onProgress) {
    // Store large file in chunks (prevent memory issues)
    // Support progress callbacks
  }

  async getFile(name) {
    // Retrieve file blob from IndexedDB
  }

  async hasFile(name) {
    // Check if file exists (for first-time check)
  }

  async deleteFile(name) {
    // Allow users to re-upload if needed
  }
}
```

#### C. Emscripten File System Integration

**File:** `Build_Release/asset-loader.js` (new file)

Bridge IndexedDB to Emscripten's virtual file system:
```javascript
async function mountAssetStorage() {
  const storage = new AssetStorage();
  await storage.init();

  // Check if CLAW.REZ exists in IndexedDB
  const hasClawRez = await storage.hasFile('CLAW.REZ');

  if (!hasClawRez) {
    // Show upload UI
    showAssetUpload();
    await waitForUpload();
  }

  // Mount IndexedDB as Emscripten virtual FS
  // Use IDBFS (Emscripten's IndexedDB filesystem)
  FS.mkdir('/assets');
  FS.mount(IDBFS, {}, '/assets');

  // Sync from IndexedDB to virtual FS
  await new Promise((resolve, reject) => {
    FS.syncfs(true, (err) => err ? reject(err) : resolve());
  });
}
```

**Integration Points:**
- Hook into Module.preRun (line 532 in openclaw.html)
- Call `mountAssetStorage()` before game initialization
- Update game to read from `/assets/CLAW.REZ` path

---

### Phase 3: Lazy Asset Loading Architecture

**Goal:** Load assets on-demand instead of bundling everything into openclaw.data.

#### A. Update CMakeLists.txt

**File:** `CMakeLists.txt` (line 98)

Change from monolithic preload to minimal bundle:
```cmake
# OLD (bundles everything):
--preload-file ../Build_Release/CLAW.REZ@CLAW.REZ
--preload-file ../Build_Release/ASSETS.ZIP@ASSETS.ZIP

# NEW (minimal critical assets only):
--preload-file ../Build_Release/ASSETS.ZIP@ASSETS.ZIP
--preload-file ../Build_Release/console02.tga@console02.tga
--preload-file ../Build_Release/clacon.ttf@clacon.ttf
# CLAW.REZ loaded separately from IndexedDB
```

**Benefit:** openclaw.data shrinks from 113MB to <1MB

#### B. Progressive Resource Loading

**File:** `OpenClaw/Engine/Resource/ResourceCache.cpp`

Enhance existing preload system to be async and granular:

**Current code** (line 496):
```cpp
int32 ResourceCache::Preload(const std::string pattern, void(*progressCallback)(int32, bool &))
{
    // Currently loads all matching files synchronously
}
```

**Enhancement approach:**
1. Keep existing synchronous API for compatibility
2. Add async variant that yields to browser event loop
3. Use Emscripten's `emscripten_sleep()` to prevent blocking
4. Wire up progress callbacks to JavaScript

**JavaScript bridge** (new file: `Build_Release/resource-loader.js`):
```javascript
Module.onResourceLoadProgress = function(resourceName, loaded, total) {
  // Update loading UI
  updateLoadingBar(resourceName, (loaded / total) * 100);
};
```

#### C. Level-Based Loading Strategy

**File:** `OpenClaw/Engine/GameApp/BaseGameApp.cpp`

Modify initialization sequence (lines 1011-1061):

**Current (loads everything)**:
```cpp
m_pResourceMgr->VPreload("/CLAW/*", NULL, ORIGINAL_RESOURCE);
m_pResourceMgr->VPreload("/GAME/*", NULL, ORIGINAL_RESOURCE);
m_pResourceMgr->VPreload("/STATES/*", NULL, ORIGINAL_RESOURCE);
```

**New (loads only critical assets)**:
```cpp
// Critical: Menu and UI assets only
m_pResourceMgr->VPreload("/STATES/MENU/*", progressCallback, ORIGINAL_RESOURCE);
m_pResourceMgr->VPreload("/GAME/FONTS/*", progressCallback, ORIGINAL_RESOURCE);
m_pResourceMgr->VPreload("/GAME/IMAGES/MENU/*", progressCallback, ORIGINAL_RESOURCE);

// Level assets loaded on-demand in LoadLevel()
```

**Add level preloading** (in level loading code):
```cpp
void BaseGameLogic::LoadLevel(int levelNum) {
    std::string levelPath = "/LEVEL" + std::to_string(levelNum) + "/*";
    m_pResourceMgr->VPreload(levelPath, progressCallback, ORIGINAL_RESOURCE);
    // ... rest of level loading
}
```

---

### Phase 4: Enhanced Documentation

**Goal:** Clear, comprehensive setup instructions with legal guidance.

#### A. Create SETUP.md

**File:** `SETUP.md` (new file in root)

```markdown
# OpenClaw WASM Setup Guide

## Overview
OpenClaw is a web-based reimplementation of Captain Claw (1997). This guide explains how to set up and run the game in your browser.

## Requirements

### 1. CLAW.REZ (Required)
**What:** Original game assets from Captain Claw (1997)
**Size:** ~113 MB
**Legal:** You must own the original game to use this file

**Where to obtain:**
- **Original CD/Installation:** You must own a legitimate copy of Captain Claw (1997)

**How to extract from original game:**
1. Install Captain Claw (1997) from your original disc
2. Navigate to installation directory (usually `C:\Program Files\Monolith\Claw`)
3. Copy `CLAW.REZ` file (~113 MB)
4. Upload to OpenClaw web interface

**Note:** Captain Claw is no longer available for digital purchase. You must own the original physical release.

### 2. Browser Requirements
- Modern browser with WebAssembly support (Chrome 105+, Firefox 121+, Safari 16.4+, Edge 105+)
- IndexedDB enabled (for asset storage)
- ~150 MB available storage space

## First-Time Setup

1. Open OpenClaw in your browser
2. You'll see "First Time Setup" screen
3. Click "Choose File" and select your CLAW.REZ
4. Click "Upload CLAW.REZ"
5. Wait for upload to complete (stored in browser cache)
6. Game will start automatically

**Note:** CLAW.REZ is stored locally in your browser. You only need to upload it once.

## Legal Notice

OpenClaw is an open-source game engine. The original Captain Claw game assets (CLAW.REZ) are copyrighted by Monolith Productions. You must own a legitimate copy of the original game to use these assets legally.

OpenClaw code is licensed under GNU GPL v3.

## Troubleshooting

**"Upload failed"**
- Check file is exactly named CLAW.REZ (case-insensitive)
- Verify file size is ~113 MB
- Clear browser cache and try again

**"Out of storage"**
- Check browser storage quota (Settings > Site Storage)
- Clear other site data to free space
- Try different browser

**"Assets not loading"**
- Open browser console (F12) for error messages
- Check IndexedDB is enabled in browser settings
- Try re-uploading CLAW.REZ
```

#### B. Update README.md

**File:** `README.md`

Add prominent section after title:
```markdown
## ⚠️ Important: Asset Requirements

**This is a WASM-only fork** focused on browser-based gameplay. Desktop builds are not supported.

**You will need CLAW.REZ** from the original Captain Claw (1997) game. The game will prompt you to upload this file on first run. See [SETUP.md](SETUP.md) for detailed instructions.

**Legal:** You must own the original game to use its assets. The game is no longer available for digital purchase - you must own the original physical release.
```

Update "Building for WebAssembly" section:
```markdown
## Building for WebAssembly

**Prerequisites:**
- Emscripten SDK
- Python and CMake
- **Do not include CLAW.REZ in build** (users provide at runtime)

**Quick Build:**
```bash
source ./emsdk/emsdk_env.sh
./build_wasm.sh
```

The build script automatically regenerates ASSETS.ZIP from source.
CLAW.REZ is loaded from user's browser storage at runtime.
```

#### C. Add Browser Compatibility Notice

**File:** `Build_Release/openclaw.html`

Add feature detection:
```javascript
function checkBrowserSupport() {
  const required = {
    wasm: typeof WebAssembly !== 'undefined',
    indexeddb: 'indexedDB' in window,
    file: 'File' in window && 'Blob' in window,
  };

  const unsupported = Object.entries(required)
    .filter(([name, supported]) => !supported)
    .map(([name]) => name);

  if (unsupported.length > 0) {
    alert(`Your browser doesn't support: ${unsupported.join(', ')}\n\nPlease use a modern browser (Chrome 105+, Firefox 121+, Safari 16.4+, Edge 105+)`);
    return false;
  }
  return true;
}
```

---

### Phase 5: Build Script Improvements

**Goal:** Ensure build process reflects new architecture.

**File:** `build_wasm.sh`

Update to skip CLAW.REZ checks and optimize for lazy loading:
```bash
#!/bin/bash
set -e

echo "=== OpenClaw WASM Build (Lazy Loading) ==="

# Check prerequisites
if [ ! -f "Build_Release/ASSETS.ZIP" ] && [ ! -d "Build_Release/ASSETS" ]; then
    echo "Error: ASSETS.ZIP or ASSETS/ directory not found"
    exit 1
fi

# Note: CLAW.REZ not needed for build (loaded at runtime)
if [ -f "Build_Release/CLAW.REZ" ]; then
    echo "⚠️  Warning: CLAW.REZ found in Build_Release/"
    echo "   This file is not bundled into openclaw.data (users provide at runtime)"
fi

# 1. Rebuild ASSETS.ZIP from source
echo "1. Rebuilding ASSETS.ZIP from source..."
cd Build_Release
rm -f ASSETS.ZIP
(cd ASSETS && zip -q -r ../ASSETS.ZIP .)
echo "   ASSETS.ZIP created: $(du -h ASSETS.ZIP | cut -f1)"
cd ..

# 2-6. Build steps (unchanged)
# ...

echo ""
echo "=== Build completed! ==="
echo "Initial download size: ~5MB (vs 160MB before)"
echo "CLAW.REZ loaded from user's browser storage (IndexedDB)"
```

---

## Critical Files Summary

**Files to modify:**
1. `CMakeLists.txt` - Remove desktop builds, update asset preloading
2. `.gitignore` - Add desktop artifact patterns
3. `README.md` - Add asset requirements, update build instructions
4. `Build_Release/openclaw.html` - Add upload UI, browser checks
5. `build_wasm.sh` - Update for lazy loading architecture
6. `OpenClaw/Engine/GameApp/BaseGameApp.cpp` - Lazy level loading
7. `OpenClaw/Engine/Resource/ResourceCache.cpp` - Async preload support

**Files to create:**
1. `SETUP.md` - Comprehensive setup guide
2. `Build_Release/asset-storage.js` - IndexedDB wrapper
3. `Build_Release/asset-loader.js` - Emscripten FS integration
4. `Build_Release/resource-loader.js` - Progress tracking bridge

**Files/directories to delete:**
1. `MidiProc/` - Windows MIDI processor
2. `ClawLauncher/` - Windows launcher
3. `Build_Release/*.exe`, `*.dll`, `*.lib`, `*.pdb` - Windows binaries
4. `build/`, `build_demo/`, `cmake-build-debug/` - Build artifacts
5. `OpenClaw/Engine/Audio/midiproc_c.c` - Windows MIDI stub

---

## Verification & Testing

**Step 1: Clean build**
```bash
rm -rf build/
source emsdk/emsdk_env.sh
./build_wasm.sh
```

**Step 2: Check file sizes**
```bash
ls -lh Build_Release/openclaw.{wasm,js,data}
# Expected:
# openclaw.wasm: ~47MB (unchanged)
# openclaw.js: ~400KB (unchanged)
# openclaw.data: <1MB (was 113MB)
```

**Step 3: Test first-time flow**
1. Start HTTP server: `./scripts/start_http3_server.sh`
2. Open in private/incognito window (fresh IndexedDB)
3. Should see "First Time Setup" screen
4. Upload CLAW.REZ from original game
5. Verify game starts after upload completes

**Step 4: Test cached flow**
1. Close and reopen browser (same profile)
2. Game should start immediately (no upload prompt)
3. Verify CLAW.REZ loaded from IndexedDB

**Step 5: Test lazy loading**
1. Open browser DevTools Network tab
2. Start game
3. Verify only ~5MB initial download
4. Load Level 2 - verify additional asset requests
5. Check IndexedDB (Application tab) - should contain CLAW.REZ

**Step 6: Test on multiple browsers**
- Chrome 105+ (primary target)
- Firefox 121+ (WebAssembly support)
- Safari 16.4+ (IndexedDB compatibility)
- Edge 105+ (Chromium-based)

**Step 7: Performance checks**
- Initial load time: <10 seconds (vs ~60 seconds before)
- Level load time: <2 seconds
- IndexedDB read speed: Should feel instant
- Memory usage: Should be reasonable (<500MB)

---

## Expected Outcomes

**Before:**
- Initial download: 160MB
- First load time: 60+ seconds
- Desktop code mixed with WASM
- Unclear setup requirements
- No asset caching

**After:**
- Initial download: ~5MB (97% reduction!)
- First load time: <10 seconds
- WASM-only focused codebase
- Clear setup documentation with legal guidance
- Assets cached forever in IndexedDB
- Progressive/lazy loading for optimal performance
- Modern 2026 web architecture (IndexedDB, Blob, File API)

**User Experience:**
1. User visits game URL
2. First time: Upload CLAW.REZ once (~30 seconds)
3. Subsequent visits: Instant load from cache
4. Levels load on-demand (<2 seconds each)
5. No re-downloading ever needed

This architecture leverages modern browser capabilities while respecting copyright (user provides their own assets) and delivers the fast, progressive loading experience expected from 2026 web applications.
