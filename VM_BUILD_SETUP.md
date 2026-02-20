# Building OpenClaw WASM in Linux VM

## Quick Setup (Fedora ARM64)

### 1. Install Dependencies

```bash
sudo dnf install -y cmake make git python3 nodejs npm wget gcc gcc-c++ \
    libstdc++-devel libstdc++-static
```

### 2. Install Emscripten SDK

```bash
cd /path/to/OpenClaw
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### 3. Build WASM

```bash
cd /path/to/OpenClaw
./build_wasm.sh
```

Build time: ~8-12 minutes (first build), ~3-5 minutes (subsequent builds)

## File Sharing with macOS

### Option 1: VMware Shared Folders

- Enable shared folders in VMware Fusion settings
- Share the OpenClaw project directory
- Build inside VM, files automatically appear on macOS

### Option 2: Network Share (SMB)

- Mount macOS folder in Linux VM
- Build directly into macOS filesystem

### Option 3: Manual Copy

```bash
# After build completes in VM
scp Build_Release/*.{wasm,js,data} user@mac-ip:/path/to/OpenClaw/Build_Release/
```

## Workflow

1. Edit code on macOS (VSCode, etc.)
2. Build in Linux VM: `./build_wasm.sh`
3. Files appear in macOS Build_Release/ via shared folder
4. Test in browser: `http://localhost:8081/openclaw.html` (Docker container)

## Performance

- **First build**: ~10 minutes (compiles all dependencies)
- **Incremental builds**: ~2-3 minutes (only changed files)
- **Full rebuild**: ~8 minutes

Much faster than Docker emulation on ARM64!
