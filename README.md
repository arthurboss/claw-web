# OpenClaw WASM - Captain Claw (1997) in Your Browser

> **WebAssembly fork** of [OpenClaw](https://github.com/pjasicek/OpenClaw) focusing on modern web technologies and browser experience

This is a WebAssembly port of Captain Claw (1997) that runs natively in your browser with no plugins required.

Based on [OpenClaw](https://github.com/pjasicek/OpenClaw) by pjasicek - original multiplatform C++ reimplementation of Captain Claw (1997).

For desktop builds (Windows, Linux, macOS, Android), visit the [original repository](https://github.com/pjasicek/OpenClaw).

## Key Features

- 🎮 **Web-native graphics:** SDL2 with WebGL rendering
- 🌐 **Modern browser UI:** Mobile-first fullscreen design with auto-detected resolution
- 🚀 **HTTP/3 support:** 30-50% faster loading with QUIC protocol and compression
- 🎵 **Audio:** SDL_Mixer with Web Audio backend
- 📱 **Responsive:** Automatic 4:3 aspect ratio with letterbox/pillarbox on widescreen
- ⚡ **No configuration:** Works out of the box - just load and play
- 🎯 **High refresh rate support:** Enjoy buttery-smooth rendering on 120Hz/144Hz+
  displays while gameplay stays perfectly consistent

## Tech Stack

- **Graphics:** SDL2 with WebGL backend
- **Audio:** SDL_Mixer
- **Physics:** Box2D
- **Data:** Tinyxml (data-driven approach)
- **Compiler:** Emscripten (C++ to WebAssembly)

## Building for WebAssembly
  
  The project can be compiled as wasm code. This code is executed by modern web browsers without any additional runtime dependencies. Compiled once this code can work on any devices with different operation systems.
  
  **Prerequisites:**

- Install [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) from official website. I'm not sure that compilation will be successful on a Windows platform. If you use Windows then don't show off, enable Windows Subsystem for Linux (WSL), download Linux-like system (for example, Ubuntu) from Windows Store and install Emscripten SDK. It should be something like:

    ```shell script
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    # Try to use latest version.
    # If it doesn't work the project was successfully compiled on 1.39.11 version.
    ```

- Make sure you have *python* and *cmake* packages. Ubuntu: `sudo apt install python cmake`.
- Make sure you have fresh `ASSETS.ZIP` and `CLAW.REZ` files in `Build_Release` directory.
  
  **Quick Build (Recommended):**

  ```shell script
    source ./emsdk/emsdk_env.sh
    ./build_wasm.sh
  ```
  
  **Manual Build:**

  ```shell script
    mkdir build
    cd build
    emcmake cmake -DEmscripten=1 ..
    cd ..
    ./patch_sdl2_shaders.sh  # Fixes WebGL shader compatibility issues
    cd build
    make
  ```
  
## Running the game

  You will need web server to run compiled project. There are 2 options:

### Option 1: HTTP/3 Server (Recommended)

  ```shell script
    ./scripts/start_http3_server.sh
    # Open https://localhost:8080/openclaw.html in your browser
  ```

  **Features:**

- ✅ HTTP/3 (QUIC protocol) for fastest loading
- ✅ HTTP/2 and HTTP/1.1 fallback
- ✅ Automatic HTTPS with self-signed certificates
- ✅ Zstd + Gzip compression (70-85% smaller files)
- ✅ Asset caching and security headers
- ✅ 30-50% faster loading than HTTP/1.1

  **Browser UI:**

- ✅ Mobile-first fullscreen design - game fills entire viewport
- ✅ Auto-detected resolution with 4:3 aspect ratio (letterbox/pillarbox on widescreen)
- ✅ Fullscreen button with dynamic icon (⛶/✕)
- ✅ No manual configuration needed - works out of the box
- ✅ Responsive to window resizing and fullscreen mode
  
### Option 2: Python HTTP/1.1 Server (Legacy)

  ```shell script
    python3 -m http.server 8080
    # Open http://localhost:8080/Build_Release/openclaw.html in your browser
  ```
  
  **Alternative Python server:**

  ```shell script
    cd Build_Release
    python -m SimpleHTTPServer 8080
    # Go to http://localhost:8080/openclaw.html
  ```
  
  **Production Deployment:**
  
  You can upload `openclaw.html`, `openclaw.js`, `openclaw.wasm` and `openclaw.data` (and `config.xml` if you build the project without `-DExtern_Config=0` parameter) files from `Build_Release` directory to any web server.
  
  **Browser Compatibility:**

- ✅ Chrome/Edge (latest)
- ✅ Firefox (latest)
- ✅ Safari (latest)
- ❌ Internet Explorer (no WebAssembly support)

  **Configuration:**

The game can be configured via `Build_Release/config.xml`. Notable options:

```xml
<GlobalOptions>
  <ShowFps>true</ShowFps>           <!-- Show FPS counter in bottom-left -->
  <GameLogicFps>60</GameLogicFps>   <!-- Game logic update rate (default: 60) -->
</GlobalOptions>
```

**FPS Display:** Enable `ShowFps` to see real-time rendering frame rate. With the fixed timestep implementation, the game renders at your monitor's refresh rate (60Hz, 120Hz, 144Hz, etc.) while game logic stays locked at 60 updates/second for consistent gameplay.

  **Known Limitations:**

- Firefox ALT key opens window menu ([browser bug](https://bugzilla.mozilla.org/show_bug.cgi?id=1292053)) - use fullscreen mode as workaround
